
#include "InvaderController.h"

#include "AIKnowledge.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"

#include "Math/MathFunctions.h"

#include <cmath>

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

void InvaderController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
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

void InvaderController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    if(g_player_one.player_state != game::PlayerState::ALIVE)
        return;

    const math::Vector& position = math::GetPosition(*m_transform);
    const float distance_to_player = std::fabs(math::Length(position - g_player_one.position));

    if(m_idle_timer > 2000 && distance_to_player < 3)
        m_states.TransitionTo(InvaderStates::ATTACKING);


    //if(m_idle_timer > 2000 && distance_to_player < 5)
    //    m_states.TransitionTo(InvaderStates::TRACKING);
}

void InvaderController::Tracking(const mono::UpdateContext& update_context)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const float distance_to_player = math::Length(g_player_one.position - position);
    if(distance_to_player < 5.0f)
    {
        m_states.TransitionTo(InvaderStates::ATTACKING);
        return;
    }

    const TrackingResult result = m_tracking_behaviour->Run(update_context.delta_ms);
    if(result == TrackingResult::NO_PATH || result == TrackingResult::AT_TARGET)
        m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::Attacking(const mono::UpdateContext& update_context)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const float distance_to_player = math::Length(g_player_one.position - position);
    if(distance_to_player > 5.0f || g_player_one.player_state != game::PlayerState::ALIVE)
    {
        //m_states.TransitionTo(InvaderStates::TRACKING);
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const float angle = math::AngleBetweenPoints(g_player_one.position, position) + math::PI_2();
    m_weapon->Fire(position, angle, update_context.timestamp);
}
