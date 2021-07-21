
#include "TurretSpawnerController.h"

#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Util/Random.h"

#include "Player/PlayerInfo.h"
#include "Factories.h"
#include "SystemContext.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

using namespace game;

TurretSpawnerController::TurretSpawnerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::TURRET, WeaponFaction::ENEMY, entity_id);
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    const TurretStateMachine::StateTable state_table = {
        TurretStateMachine::MakeState(TurretStates::IDLE, &TurretSpawnerController::ToIdle, &TurretSpawnerController::Idle, this),
        TurretStateMachine::MakeState(TurretStates::PREPARE_ATTACK, &TurretSpawnerController::ToPrepareAttack, &TurretSpawnerController::PrepareAttack, this),
        TurretStateMachine::MakeState(TurretStates::ATTACKING, &TurretSpawnerController::ToAttacking, &TurretSpawnerController::Attacking, this),
    };

    m_states.SetStateTableAndState(state_table, TurretStates::IDLE);
}

void TurretSpawnerController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void TurretSpawnerController::ToIdle()
{
    m_idle_timer = 0;
    m_sprite->SetShade(mono::Color::WHITE);
}

void TurretSpawnerController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    bool is_visible = false;

    const game::PlayerInfo* player_info = GetClosestActivePlayer(world_position);
    if(player_info)
    {
        is_visible = math::PointInsideQuad(world_position, player_info->viewport);
        m_attack_position = player_info->position;
    }

    const bool is_player_active = (player_info != nullptr);
    if(is_player_active && is_visible && m_idle_timer > 1000)
    {
        const bool transision = mono::Chance(20);
        if(transision)
            m_states.TransitionTo(TurretStates::PREPARE_ATTACK);
        
        m_idle_timer = 0;
    }
}

void TurretSpawnerController::ToPrepareAttack()
{
    m_prepare_timer = 0;
    m_sprite->SetShade(mono::Color::RED);
}

void TurretSpawnerController::PrepareAttack(const mono::UpdateContext& update_context)
{
    m_prepare_timer += update_context.delta_ms;
    if(m_prepare_timer > 500)
        m_states.TransitionTo(TurretStates::ATTACKING);
}

void TurretSpawnerController::ToAttacking()
{ }

void TurretSpawnerController::Attacking(const mono::UpdateContext& update_context)
{
    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

//    game::WeaponState fire_state = m_weapon->Fire(world_position, m_attack_position, update_context.timestamp);
//    if(fire_state == game::WeaponState::FIRE)
//        m_states.TransitionTo(TurretStates::IDLE);
}
