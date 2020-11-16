
#include "GoblinFireController.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"
#include "Util/Random.h"

#include "AIKnowledge.h"
#include "Factories.h"
#include "SystemContext.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

using namespace game;

GoblinFireController::GoblinFireController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::CACOPLASMA, WeaponFaction::ENEMY, entity_id);
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    using namespace std::placeholders;

    const std::unordered_map<GoblinStates, GoblinStateMachine::State>& state_table = {
        { GoblinStates::IDLE,       { std::bind(&GoblinFireController::ToIdle,      this), std::bind(&GoblinFireController::Idle,       this, _1) } },
        { GoblinStates::HURT,       { std::bind(&GoblinFireController::ToHurt,      this), std::bind(&GoblinFireController::Hurt,       this, _1) } },
        { GoblinStates::ATTACKING,  { std::bind(&GoblinFireController::ToAttacking, this), std::bind(&GoblinFireController::Attacking,  this, _1) } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(GoblinStates::IDLE);
}

void GoblinFireController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void GoblinFireController::ToIdle()
{
    m_idle_timer = 0;
}

void GoblinFireController::ToHurt()
{ }

void GoblinFireController::ToAttacking()
{
    m_attack_timer = 0;
    m_n_attacks = 0;
}

void GoblinFireController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
    const math::Vector& world_position = math::GetPosition(world_transform);

    const bool is_visible = math::PointInsideQuad(world_position, g_camera_viewport);
    if(is_visible && m_idle_timer > 500)
    {
        const bool percentage = mono::Random();
        if(percentage <= 0.2f)
            m_states.TransitionTo(GoblinStates::ATTACKING);

        m_idle_timer = 0;
    }
}

void GoblinFireController::Hurt(const mono::UpdateContext& update_context)
{
}

void GoblinFireController::Attacking(const mono::UpdateContext& update_context)
{
    if(!g_player_one.is_active || m_n_attacks > 3)
        m_states.TransitionTo(GoblinStates::IDLE);

    m_attack_timer += update_context.delta_ms;
    if(m_attack_timer > 100)
    {
        const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
        const math::Vector& world_position = math::GetPosition(world_transform);

        const float angle = math::AngleBetweenPoints(g_player_one.position, world_position) + math::PI_2();
        m_weapon->Fire(world_position, angle, update_context.timestamp);

        m_n_attacks++;
        m_attack_timer = 0;
    }
}
