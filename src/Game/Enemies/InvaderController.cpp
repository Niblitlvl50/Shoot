
#include "InvaderController.h"

#include "AIKnowledge.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeaponSystem.h"
#include "Weapons/IWeaponFactory.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"

#include "Math/MathFunctions.h"

using namespace game;

InvaderController::InvaderController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(WeaponType::GENERIC, WeaponFaction::ENEMY, entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* entity_body = physics_system->GetBody(entity_id);

    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(entity_body, physics_system);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    using namespace std::placeholders;

    const std::unordered_map<InvaderStates, InvaderStateMachine::State>& state_table = {
        { InvaderStates::IDLE,      { std::bind(&InvaderController::ToIdle,      this), std::bind(&InvaderController::Idle,      this, _1) } },
        { InvaderStates::TRACKING,  { std::bind(&InvaderController::ToTracking,  this), std::bind(&InvaderController::Tracking,  this, _1) } },
        { InvaderStates::ATTACKING, { std::bind(&InvaderController::ToAttacking, this), std::bind(&InvaderController::Attacking, this, _1) } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(InvaderStates::IDLE);
}

InvaderController::~InvaderController()
{ }

void InvaderController::Update(uint32_t delta_ms)
{
    m_states.UpdateState(delta_ms);
}

void InvaderController::ToIdle()
{
    constexpr mono::Color::RGBA color(0.2, 0.2, 0.2);
    m_sprite->SetShade(color);

    m_idle_timer = 0;
}

void InvaderController::ToTracking()
{
    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f);
    m_sprite->SetShade(color);
}

void InvaderController::ToAttacking()
{

}

void InvaderController::Idle(uint32_t delta_ms)
{
    m_idle_timer += delta_ms;
    if(m_idle_timer > 2000 && g_player_one.is_active)
        m_states.TransitionTo(InvaderStates::TRACKING);
}

void InvaderController::Tracking(uint32_t delta_ms)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const float distance_to_player = math::Length(g_player_one.position - position);
    if(distance_to_player < 5.0f)
    {
        m_states.TransitionTo(InvaderStates::ATTACKING);
        return;
    }

    const TrackingResult result = m_tracking_behaviour->Run(delta_ms);
    if(result == TrackingResult::NO_PATH || result == TrackingResult::AT_TARGET)
        m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::Attacking(uint32_t delta_ms)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const float distance_to_player = math::Length(g_player_one.position - position);
    if(distance_to_player > 10.0f || !g_player_one.is_active)
    {
        m_states.TransitionTo(InvaderStates::TRACKING);
        return;
    }

    const float angle = math::AngleBetweenPoints(g_player_one.position, position) + math::PI_2();
    m_weapon->Fire(position, angle);
}
