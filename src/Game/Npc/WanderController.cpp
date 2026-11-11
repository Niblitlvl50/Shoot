
#include "WanderController.h"

#include "Debug/IDebugDrawer.h"
#include "Entity/TargetSystem.h"
#include "Navigation/NavigationSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"
#include "Paths/PathSystem.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Color.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/SpriteProperties.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Random.h"

#include <cstdio>
#include <cstring>

namespace tweak_values
{
    constexpr float target_check_interval_s = 1.25f;

    // How close to the player it stops when approaching.
    constexpr float approach_stop_distance = 0.25f;

    // The player has to get this much further away than the approach distance before it's let go.
    constexpr float approach_release_margin = 0.5f;

    // Gives up on a point it can't reach, blocked by something for example.
    constexpr float max_wander_time_s = 10.0f;
}

using namespace game;

WanderController::WanderController(uint32_t entity_id, const WanderSettings& settings, mono::SystemContext* system_context)
    : m_entity_id(entity_id)
    , m_settings(settings)
    , m_body(nullptr)
    , m_sprite(nullptr)
    , m_animation_name(nullptr)
    , m_current_point_index(-1)
    , m_approach_done(false)
    , m_pause_time_s(0.0f)
    , m_target_check_timer_s(0.0f)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_path_system = system_context->GetSystem<mono::PathSystem>();
    m_target_system = system_context->GetSystem<game::TargetSystem>();

    mono::PhysicsSystem* physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    //if(physics_system->IsAllocated(entity_id))
        m_body = physics_system->GetBody(entity_id);

    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    if(sprite_system->IsAllocated(entity_id))
        m_sprite = sprite_system->GetSprite(entity_id);

    if(m_body)
    {
        m_tracking_behaviour.Init(m_body, system_context->GetSystem<game::NavigationSystem>());
        m_tracking_behaviour.SetTrackingSpeed(m_settings.speed);
    }

    const MyStateMachine::StateTable state_table = {
        MyStateMachine::MakeState(States::PAUSE,    &WanderController::ToPause,     &WanderController::PauseState, this),
        MyStateMachine::MakeState(States::WANDER,   &WanderController::ToWander,    &WanderController::WanderState, this),
        MyStateMachine::MakeState(States::APPROACH, &WanderController::ToApproach,  &WanderController::ApproachState, this),
    };
    m_states.SetStateTableAndState(state_table, States::PAUSE);
}

void WanderController::Update(const mono::UpdateContext& update_context)
{
    if(!m_body)
        return;

    // The path entity might not be set up when the logic is created, its points are read here instead.
    if(m_wander_points.empty())
        ReadWanderPoints();

    const States active_state = m_states.ActiveState();
    if(m_settings.approach_distance > 0.0f && active_state != States::APPROACH)
    {
        m_target_check_timer_s += update_context.delta_s;
        if(m_target_check_timer_s >= tweak_values::target_check_interval_s)
        {
            m_target_check_timer_s = 0.0f;

            const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
            m_target = m_target_system->AquireTarget(TargetFaction::Player, position, m_settings.approach_distance);
            if(m_target && m_target->IsValid())
                m_states.TransitionTo(States::APPROACH);
        }
    }

    m_states.UpdateState(update_context);

    if(m_sprite)
    {
        const math::Vector& velocity = m_body->GetVelocity();
        if(velocity.x < 0.0f)
            m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
        else if(velocity.x > 0.0f)
            m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    }
}

void WanderController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);

    for(const math::Vector& point : m_wander_points)
        debug_drawer->DrawPoint(point, 8.0f, mono::Color::CYAN);

    if(m_settings.approach_distance > 0.0f)
        debug_drawer->DrawCircle(position, m_settings.approach_distance, mono::Color::MAGENTA);

    const States active_state = m_states.ActiveState();
    const bool is_walking = (active_state == States::WANDER) || (active_state == States::APPROACH && !m_approach_done);
    if(is_walking)
        debug_drawer->DrawLine(position, m_tracking_behaviour.GetPathEndPoint(), 1.0f, mono::Color::GREEN);

    const char* state_name = "Pause";
    if(active_state == States::WANDER)
        state_name = "Wander";
    else if(active_state == States::APPROACH)
        state_name = "Approach";

    debug_drawer->DrawWorldText(state_name, position, mono::Color::OFF_WHITE);
}

const char* WanderController::GetDebugCategory() const
{
    return "wander controller";
}

void WanderController::ToPause()
{
    StandStill();
    m_pause_time_s = mono::Random(m_settings.pause_interval.min, m_settings.pause_interval.max);
    m_target = nullptr;

    PlayAnimation("idle");
}

void WanderController::PauseState(const mono::UpdateContext& update_context)
{
    m_pause_time_s -= update_context.delta_s;
    if(m_pause_time_s <= 0.0f)
        m_states.TransitionTo(States::WANDER);
}

void WanderController::ToWander()
{
    if(m_wander_points.empty())
    {
        m_states.TransitionTo(States::PAUSE);
        return;
    }

    // A random point, but not the one it's at.
    const int n_points = static_cast<int>(m_wander_points.size());
    int point_index = mono::RandomInt(0, n_points - 1);
    if(point_index == m_current_point_index && n_points > 1)
        point_index = (point_index + 1) % n_points;

    m_current_point_index = point_index;

    // No path there, try another point after the next pause.
    if(!m_tracking_behaviour.UpdatePath(m_wander_points[point_index]))
    {
        m_states.TransitionTo(States::PAUSE);
        return;
    }

    m_pause_time_s = tweak_values::max_wander_time_s;
    PlayAnimation("walk");
}

void WanderController::WanderState(const mono::UpdateContext& update_context)
{
    // Counts down the time it may spend walking.
    m_pause_time_s -= update_context.delta_s;

    const TrackingResult result = m_tracking_behaviour.Run(update_context);
    if(result.state != TrackingState::TRACKING || m_pause_time_s <= 0.0f)
        m_states.TransitionTo(States::PAUSE);
}

void WanderController::ToApproach()
{
    if(!m_target || !m_target->IsValid())
    {
        m_states.TransitionTo(States::PAUSE);
        return;
    }

    // It's not at a point anymore, so any point can be next.
    m_current_point_index = -1;

    // Walks up to where the player is now, once, stopping a bit in front of them.
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector target_position = m_target->Position();
    const math::Vector to_target = target_position - position;
    const float distance_to_target = math::Length(to_target);

    m_approach_done = (distance_to_target <= tweak_values::approach_stop_distance);
    if(!m_approach_done)
    {
        const math::Vector approach_point =
            target_position - (to_target / distance_to_target) * tweak_values::approach_stop_distance;
        m_approach_done = !m_tracking_behaviour.UpdatePath(approach_point);
    }

    if(!m_approach_done)
        PlayAnimation("walk");
}

void WanderController::ApproachState(const mono::UpdateContext& update_context)
{
    const math::Vector position = m_transform_system->GetWorldPosition(m_entity_id);

    // Stays until the player leaves, then goes back to wandering.
    const float release_distance = m_settings.approach_distance + tweak_values::approach_release_margin;
    if(!m_target || !m_target->IsValid() || !m_target->IsWithinDistance(position, release_distance))
    {
        m_states.TransitionTo(States::PAUSE);
        return;
    }

    if(!m_approach_done)
    {
        const bool close_to_player = m_target->IsWithinDistance(position, tweak_values::approach_stop_distance);
        const TrackingResult result = m_tracking_behaviour.Run(update_context);
        m_approach_done = close_to_player || result.state != TrackingState::TRACKING;
        if(!m_approach_done)
            return;
    }

    // Standing where the approach ended, facing the player.
    const math::Vector target_position = m_target->Position();
    StandStill();
    PlayAnimation("idle");

    if(m_sprite)
    {
        if(target_position.x < position.x)
            m_sprite->SetProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
        else
            m_sprite->ClearProperty(mono::SpriteProperty::FLIP_HORIZONTAL);
    }
}

void WanderController::StandStill()
{
    if(m_body)
        m_body->SetVelocity(math::ZeroVec);
}

void WanderController::PlayAnimation(const char* animation_name)
{
    // Setting an animation restarts it, so only when it changes.
    if(!m_sprite || (m_animation_name && std::strcmp(m_animation_name, animation_name) == 0))
        return;

    m_sprite->SetAnimation(animation_name);
    m_animation_name = animation_name;
}

void WanderController::ReadWanderPoints()
{
    const uint32_t path_entity_id = m_entity_manager->GetEntityIdFromUuid(m_settings.path_entity_uuid);
    if(path_entity_id == mono::INVALID_ID)
        return;

    const mono::PathComponent* path_component = m_path_system->GetPath(path_entity_id);
    if(!path_component)
        return;

    const math::Matrix& path_transform = m_transform_system->GetWorld(path_entity_id);
    for(const math::Vector& local_point : path_component->points)
        m_wander_points.push_back(math::Transformed(path_transform, local_point));
}
