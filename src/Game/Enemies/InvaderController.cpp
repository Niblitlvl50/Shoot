
#include "InvaderController.h"

#include "Player/PlayerInfo.h"
#include "Behaviour/TrackingBehaviour.h"
#include "Factories.h"
#include "Weapons/IWeapon.h"
#include "Weapons/IWeaponFactory.h"

#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"

#include "Math/MathFunctions.h"

#include <cmath>

namespace tweak_values
{
    constexpr uint32_t idle_time = 2000;
    constexpr float attack_distance = 3.0f;
}

using namespace game;

InvaderController::InvaderController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_entity_id(entity_id)
{
    m_weapon = g_weapon_factory->CreateWeapon(game::GENERIC, WeaponFaction::ENEMY, entity_id);

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* entity_body = physics_system->GetBody(entity_id);

    m_tracking_behaviour = std::make_unique<TrackingBehaviour>(entity_body, physics_system);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    mono::TransformSystem* transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_transform = &transform_system->GetTransform(entity_id);

    const InvaderStateMachine::StateTable& state_table = {
        InvaderStateMachine::MakeState(InvaderStates::IDLE, &InvaderController::ToIdle, &InvaderController::Idle, this),
        InvaderStateMachine::MakeState(InvaderStates::TRACKING, &InvaderController::ToTracking, &InvaderController::Tracking, this),
        InvaderStateMachine::MakeState(InvaderStates::ATTACKING, &InvaderController::ToAttacking, &InvaderController::Attacking, this),
    };
    m_states.SetStateTableAndState(state_table, InvaderStates::IDLE);
}

InvaderController::~InvaderController()
{ }

void InvaderController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void InvaderController::ToIdle()
{
    m_idle_timer = 0;
}

void InvaderController::Idle(const mono::UpdateContext& update_context)
{
    m_idle_timer += update_context.delta_ms;

    const math::Vector& position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
        return;

    if(m_idle_timer > tweak_values::idle_time)
    {
        const float distance_to_player = math::DistanceBetween(position, player_info->position);
        if(distance_to_player < tweak_values::attack_distance)
            m_states.TransitionTo(InvaderStates::ATTACKING);
        else if(distance_to_player > 1.0f && distance_to_player < 10.0f)
            m_states.TransitionTo(InvaderStates::TRACKING);
    }
}

void InvaderController::ToTracking()
{ }

void InvaderController::Tracking(const mono::UpdateContext& update_context)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const math::Vector delta = position - player_info->position;
    const bool left_of_player = (delta.x < 0.0f);
    if(left_of_player)
        m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    else
        m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);

    const float distance_to_player = math::DistanceBetween(position, player_info->position);
    if(distance_to_player < 1.0f)
    {
        m_states.TransitionTo(InvaderStates::ATTACKING);
        return;
    }

    const TrackingResult result = m_tracking_behaviour->Run(update_context, player_info->position);
    if(result == TrackingResult::NO_PATH || result == TrackingResult::AT_TARGET)
        m_states.TransitionTo(InvaderStates::IDLE);
}

void InvaderController::ToAttacking()
{ }

void InvaderController::Attacking(const mono::UpdateContext& update_context)
{
    const math::Vector& position = math::GetPosition(*m_transform);
    const game::PlayerInfo* player_info = GetClosestActivePlayer(position);
    if(!player_info)
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const float distance_to_player = math::DistanceBetween(player_info->position, position);
    if(distance_to_player > 5.0f)
    {
        m_states.TransitionTo(InvaderStates::IDLE);
        return;
    }

    const float angle = math::AngleBetweenPoints(player_info->position, position) + math::PI_2();
    m_weapon->Fire(position, angle, update_context.timestamp);
}
