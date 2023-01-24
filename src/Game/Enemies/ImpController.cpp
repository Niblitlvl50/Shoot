
#include "ImpController.h"
#include "AIUtils.h"

#include "Physics/PhysicsSystem.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "Util/Random.h"
#include "Debug/IDebugDrawer.h"

#include "Player/PlayerInfo.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/WeaponSystem.h"

#include "EntitySystem/IEntityManager.h"


namespace tweak_values
{
    constexpr float activate_distance_to_player_threshold = 4.0f;
    constexpr float perpendicular_movement_distance_threshold = 2.0f;
    constexpr float move_speed = 0.5f;
    constexpr float degrees_per_second = 360.0f;

    //constexpr uint32_t attack_start_delay = 500;
    constexpr float attack_sequence_delay_s = 0.05f;
    constexpr uint32_t n_attacks = 3;
}

using namespace game;

ImpController::ImpController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_entity_system = system_context->GetSystem<mono::IEntityManager>();

    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    m_weapon = weapon_system->CreatePrimaryWeapon(entity_id, WeaponFaction::ENEMY);

    mono::IBody* body = m_physics_system->GetBody(entity_id);

    m_homing_behaviour.SetBody(body);
    m_homing_behaviour.SetForwardVelocity(tweak_values::move_speed);
    m_homing_behaviour.SetAngularVelocity(tweak_values::degrees_per_second);

    m_sprite = m_sprite_system->GetSprite(entity_id);

    m_idle_anim_id = m_sprite->GetAnimationIdFromName("idle");
    m_run_anim_id = m_sprite->GetAnimationIdFromName("run");
    m_attack_anim_id = m_sprite->GetAnimationIdFromName("attack");

    const mono::Entity spawned_weapon = m_entity_system->CreateEntity("res/entities/player_weapon.entity");
    m_transform_system->ChildTransform(spawned_weapon.id, m_entity_id);
    m_weapon_entity = spawned_weapon.id;

    const GoblinStateMachine::StateTable state_table = {
        GoblinStateMachine::MakeState(States::IDLE, &ImpController::ToIdle, &ImpController::Idle, this),
        GoblinStateMachine::MakeState(States::REPOSITION, &ImpController::ToReposition, &ImpController::Reposition, this),
        GoblinStateMachine::MakeState(States::PREPARE_ATTACK, &ImpController::ToPrepareAttack, &ImpController::PrepareAttack, this),
        GoblinStateMachine::MakeState(States::ATTACKING, &ImpController::ToAttacking, &ImpController::Attacking, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

ImpController::~ImpController()
{
    m_entity_system->ReleaseEntity(m_weapon_entity);
}

void ImpController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);

    UpdateWeaponAnimation(update_context);
}

void ImpController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    debug_drawer->DrawCircle(world_position, tweak_values::activate_distance_to_player_threshold, mono::Color::CYAN);
    debug_drawer->DrawCircle(world_position, tweak_values::perpendicular_movement_distance_threshold, mono::Color::GREEN);

    const math::Vector& target_position = m_homing_behaviour.GetTargetPosition();
    debug_drawer->DrawLine({ world_position, target_position }, 1.0f, mono::Color::BLUE);
    debug_drawer->DrawPoint(target_position, 10.0f, mono::Color::BLUE);

    if(m_target_player)
    {
        debug_drawer->DrawLine({ world_position, m_target_player->position }, 1.0f, mono::Color::RED);
        debug_drawer->DrawPoint(m_target_player->position, 5.0f, mono::Color::RED);
    }

    const char* state_string = nullptr;
    switch(m_states.ActiveState())
    {
    case States::IDLE:
        state_string = "Idle";
        break;
    case States::REPOSITION:
        state_string = "Reposition";
        break;
    case States::PREPARE_ATTACK:
        state_string = "Prepare Attack";
        break;
    case States::ATTACKING:
        state_string = "Attacking";
        break;
    }

    debug_drawer->DrawWorldText(state_string, world_position, mono::Color::OFF_WHITE);
}

const char* ImpController::GetDebugCategory() const
{
    return "Imp";
}

void ImpController::ToIdle()
{
    m_idle_timer_s = 0.0f;
    m_sprite->SetShade(mono::Color::WHITE);
    m_sprite->SetAnimation(m_idle_anim_id);
}

void ImpController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer_s += update_context.delta_s;
    if(m_idle_timer_s < 1.0f)
        return;

    m_idle_timer_s = 0.0f;

    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

    m_target_player = GetClosestActivePlayer(
        world_position, tweak_values::activate_distance_to_player_threshold);
    if(!m_target_player)
        return;

    const bool is_left_of = (m_target_player->position.x < world_position.x);
    if(is_left_of)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else 
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const bool sees_player = game::SeesPlayer(m_physics_system, world_position, m_target_player);
    if(sees_player)
    {
        const bool transition_to_attack = mono::Chance(25);
        const States new_state = transition_to_attack ? States::PREPARE_ATTACK : States::REPOSITION;
        m_states.TransitionTo(new_state);
    }
}

void ImpController::ToReposition()
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(!player_info)
        return;

    const math::Vector delta = player_info->position - world_position;
    const math::Vector normalized_delta = math::Normalized(delta);

    math::Vector homing_target = world_position;

    const float distance_to_player = math::Length(delta);
    if(distance_to_player < tweak_values::perpendicular_movement_distance_threshold)
    {
        // Move sideways
        const float multiplier = mono::Chance(50) ? -1.0f : 1.0f;
        homing_target += math::Perpendicular(normalized_delta) * multiplier;
    }
    else
    {
        // Move towards player
        homing_target += normalized_delta;
    }

    m_sprite->SetAnimation(m_run_anim_id);
    m_homing_behaviour.SetTargetPosition(homing_target);
}

void ImpController::Reposition(const mono::UpdateContext& update_context)
{
    const game::HomingResult result = m_homing_behaviour.Run(update_context);
    if(result.distance_to_target < 0.1f)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);
        const bool transition_to_attack =
            mono::Chance(75) && game::SeesPlayer(m_physics_system, world_position, m_target_player);

        const States new_state = transition_to_attack ? States::PREPARE_ATTACK : States::IDLE;
        m_states.TransitionTo(new_state);
    }
    else if(result.is_stuck)
    {
        m_states.TransitionTo(States::IDLE);
    }
}

void ImpController::ToPrepareAttack()
{
    const auto transition_to_attacking = [this](uint32_t sprite_id) {
        m_states.TransitionTo(States::ATTACKING);
    };
    m_sprite->SetAnimation(m_attack_anim_id, transition_to_attacking);

    m_attack_position = m_target_player->position;

    // Show something here to indicate attack direction
}

void ImpController::PrepareAttack(const mono::UpdateContext& update_context)
{ }

void ImpController::ToAttacking()
{
    m_attack_timer_s = 0.0f;
    m_n_attacks = 0;
}

void ImpController::Attacking(const mono::UpdateContext& update_context)
{
    if(m_n_attacks >= tweak_values::n_attacks)
        m_states.TransitionTo(States::IDLE);
    
    if(m_weapon->UpdateWeaponState(update_context.timestamp) == game::WeaponState::RELOADING)
        m_states.TransitionTo(States::IDLE);

    m_attack_timer_s += update_context.delta_s;
    if(m_attack_timer_s > tweak_values::attack_sequence_delay_s)
    {
        const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

        game::WeaponState fire_state = m_weapon->Fire(world_position, m_attack_position, update_context.timestamp);
        if(fire_state == game::WeaponState::FIRE)
            m_n_attacks++;
        else if(fire_state == game::WeaponState::OUT_OF_AMMO)
            m_weapon->Reload(update_context.timestamp);
    
        m_attack_timer_s = 0.0f;
    }
}

void ImpController::UpdateWeaponAnimation(const mono::UpdateContext& update_context)
{
    if(!m_target_player)
        return;

    const math::Vector world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const float m_aim_direction = math::AngleBetweenPointsSimple(world_position, m_attack_position);

    mono::Sprite* weapon_sprite = m_sprite_system->GetSprite(m_weapon_entity);
    if(m_aim_direction < 0.0f)
        weapon_sprite->SetProperty(mono::SpriteProperty::FLIP_VERTICAL);
    else
        weapon_sprite->ClearProperty(mono::SpriteProperty::FLIP_VERTICAL);

    math::Matrix& weapon_transform = m_transform_system->GetTransform(m_weapon_entity);
    weapon_transform =
        math::CreateMatrixWithPosition(math::Vector(0.0f, -0.1f)) *
        math::CreateMatrixFromZRotation(m_aim_direction) *
        math::CreateMatrixWithPosition(math::Vector(0.1f, 0.0f));
}
