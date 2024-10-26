
#include "BombThrowerController.h"

#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Weapons/WeaponSystem.h"
#include "Weapons/IWeapon.h"

#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"
#include "SystemContext.h"

namespace tweak_values
{
    constexpr float targeting_range = 4.0f;
    constexpr float attack_range = 3.5f;

    constexpr float idle_timer_s = 2.0f;
    //constexpr float attack_start_delay_s = 0.5f;
    constexpr float attack_sequence_delay_s = 0.5f;
    constexpr int n_attacks = 1;
}

using namespace game;

BombThrowerController::BombThrowerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    m_idle_id = sprite->GetAnimationIdFromName("idle");
    m_gesture_id = sprite->GetAnimationIdFromName("gesture");
    m_attack_id = sprite->GetAnimationIdFromName("attack");

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, game::WeaponFaction::ENEMY);

    const TStateMachine::StateTable state_table = {
        TStateMachine::MakeState(States::IDLE, &BombThrowerController::EnterIdle, &BombThrowerController::Idle, this),
        TStateMachine::MakeState(States::PREPARE_ATTACK, &BombThrowerController::EnterPrepareAttack, &BombThrowerController::PrepareAttack, this),
        TStateMachine::MakeState(States::ATTACK, &BombThrowerController::EnterAttack, &BombThrowerController::Attack, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

BombThrowerController::~BombThrowerController() = default;

void BombThrowerController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_weapon->UpdateWeaponState(update_context.timestamp);
}

void BombThrowerController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::targeting_range, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::attack_range, mono::Color::RED);
}

const char* BombThrowerController::GetDebugCategory() const
{
    return "BombThrower";
}

void BombThrowerController::EnterIdle()
{
    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetAnimation(m_idle_id);

    m_idle_timer_s = 0.0f;
}

void BombThrowerController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;

    if(m_idle_timer_s < tweak_values::idle_timer_s)
        return;

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    m_aquired_target = m_target_system->AquireTarget(TargetFaction::Player, world_position, tweak_values::targeting_range);

    if(!m_aquired_target->IsValid())
    {
        m_idle_timer_s = 0.0f;
        return;
    }

    const math::Vector& target_position = m_aquired_target->Position();

    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    const bool is_left_of = (target_position.x < world_position.x);
    if(is_left_of)
        sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else 
        sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const bool is_within_range = m_aquired_target->IsWithinRange(world_position, tweak_values::attack_range);
    if(is_within_range)
    {
        m_states.TransitionTo(States::PREPARE_ATTACK);
    }

    m_idle_timer_s = 0.0f;
}

void BombThrowerController::EnterPrepareAttack()
{
    const mono::SpriteAnimationCallback& transision_to_attack = [this](uint32_t sprite_id) {
        m_states.TransitionTo(States::ATTACK);
    };

    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetAnimation(m_gesture_id, transision_to_attack);
}

void BombThrowerController::PrepareAttack(const mono::UpdateContext& update_context)
{
    // Nothing, just waiting for the animation to finish
}

void BombThrowerController::EnterAttack()
{
    mono::Sprite* sprite = m_sprite_system->GetSprite(m_entity_id);
    sprite->SetAnimation(m_attack_id);

    m_attack_timer_s = 0.0f;
    m_n_attacks = 0;
}

void BombThrowerController::Attack(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks)
        m_states.TransitionTo(States::IDLE);

    m_attack_timer_s += update_context.delta_s;
    if(m_attack_timer_s > tweak_values::attack_sequence_delay_s)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
        const math::Vector& target_position = m_aquired_target->Position();

        game::WeaponState fire_state = m_weapon->Fire(world_position, target_position, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    
        m_attack_timer_s = 0.0f;
    }
}
