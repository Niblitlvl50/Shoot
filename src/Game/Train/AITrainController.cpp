
#include "AITrainController.h"
#include "TrainCarSystem.h"
#include "Behaviour/PathFollowerSystem.h"
#include "RailwaySystem/RailwaySystem.h"
#include "Debug/IDebugDrawer.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/Entity.h"
#include "Math/MathFunctions.h"
#include "Rendering/Color.h"
#include "Util/Random.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>

namespace tweak_values
{
    constexpr float cruise_throttle = 0.7f;
    constexpr float accelerate_per_s = 0.35f;
    constexpr float brake_per_s = 0.8f;

    // Braking for something ahead starts at brake_distance and holds at least creep_throttle until it's
    // within stop_distance, so the train rolls up to it instead of stopping short.
    constexpr float brake_distance = 6.0f;
    constexpr float creep_throttle = 0.12f;
    constexpr float stop_distance = 0.4f;

    // Something counts as on the track ahead when it's within this far to either side.
    constexpr float track_width = 1.2f;

    constexpr float station_wait_s = 4.0f;
    constexpr float station_leave_distance = 4.0f;
    constexpr float dead_end_wait_s = 1.5f;

    constexpr float train_keep_distance = 2.5f;
    constexpr float blocked_clear_wait_s = 1.0f;

    // How close to a switch the train decides on a branch, and the chance (percent) it takes the other one.
    constexpr float branch_decision_distance = 5.0f;
    constexpr int switch_toggle_chance = 50;
}

using namespace game;

AITrainController::AITrainController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
    , m_direction(1.0f)
    , m_throttle(0.0f)
    , m_state_timer_s(0.0f)
    , m_target_station_id(mono::INVALID_ID)
    , m_last_station_id(mono::INVALID_ID)
    , m_branch_decided_on_track_id(mono::INVALID_ID)
{
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_path_follower_system = system_context->GetSystem<game::PathFollowerSystem>();
    m_railway_system = system_context->GetSystem<game::RailwaySystem>();
    m_train_car_system = system_context->GetSystem<game::TrainCarSystem>();

    const TrainStateMachine::StateTable state_table = {
        TrainStateMachine::MakeState(States::DRIVING,            &AITrainController::ToDriving,          &AITrainController::Driving,          this),
        TrainStateMachine::MakeState(States::STOPPED_AT_STATION, &AITrainController::ToStoppedAtStation, &AITrainController::StoppedAtStation, this),
        TrainStateMachine::MakeState(States::DEAD_END,           &AITrainController::ToDeadEnd,          &AITrainController::DeadEnd,          this),
        TrainStateMachine::MakeState(States::BLOCKED,            &AITrainController::ToBlocked,          &AITrainController::Blocked,          this),
    };
    m_states.SetStateTableAndState(state_table, States::DRIVING);
}

void AITrainController::Update(const mono::UpdateContext& update_context)
{
    // Not on a track yet, the path reference is resolved later.
    if(m_path_follower_system->GetCurrentPathEntity(m_entity_id) == mono::INVALID_ID)
        return;

    m_states.UpdateState(update_context);
}

void AITrainController::ToDriving()
{
    m_target_station_id = mono::INVALID_ID;
}

void AITrainController::Driving(const mono::UpdateContext& update_context)
{
    // Forget the last station once it's been left behind, so it's stopped at the next time around.
    if(m_last_station_id != mono::INVALID_ID)
    {
        const math::Vector train_position = m_transform_system->GetWorldPosition(m_entity_id);
        const math::Vector station_position = m_transform_system->GetWorldPosition(m_last_station_id);
        if(math::DistanceBetween(train_position, station_position) > tweak_values::station_leave_distance)
            m_last_station_id = mono::INVALID_ID;
    }

    const float distance_to_train = DistanceToTrainAhead();
    if(distance_to_train >= 0.0f && distance_to_train < tweak_values::train_keep_distance)
    {
        m_states.TransitionTo(States::BLOCKED);
        return;
    }

    // Before checking for a dead end, the branch it picks decides what's ahead.
    DecideBranchAhead();

    uint32_t station_id = mono::INVALID_ID;
    const float distance_to_station = DistanceToStationAhead(station_id);

    // The track end only matters when nothing continues from it.
    const bool forward = (m_direction > 0.0f);
    const bool dead_end_ahead = !m_railway_system->HasTrackAhead(m_entity_id, forward);
    const float distance_to_end = dead_end_ahead ? DistanceToTrackEnd() : std::numeric_limits<float>::max();

    if(distance_to_station >= 0.0f && distance_to_station < tweak_values::stop_distance)
    {
        m_target_station_id = station_id;
        m_states.TransitionTo(States::STOPPED_AT_STATION);
        return;
    }

    if(dead_end_ahead && IsAtTrackEnd())
    {
        m_states.TransitionTo(States::DEAD_END);
        return;
    }

    // Brake for whatever comes first.
    float stop_ahead = std::numeric_limits<float>::max();
    if(distance_to_station >= 0.0f)
        stop_ahead = std::min(stop_ahead, distance_to_station);
    if(distance_to_train >= 0.0f)
        stop_ahead = std::min(stop_ahead, distance_to_train - tweak_values::train_keep_distance);
    stop_ahead = std::min(stop_ahead, distance_to_end);

    float target_throttle = tweak_values::cruise_throttle;
    if(stop_ahead < tweak_values::brake_distance)
    {
        const float brake_fraction = std::clamp(stop_ahead / tweak_values::brake_distance, 0.0f, 1.0f);
        target_throttle = std::max(tweak_values::cruise_throttle * brake_fraction, tweak_values::creep_throttle);
    }

    ApplyThrottle(target_throttle, update_context.delta_s);
}

void AITrainController::DecideBranchAhead()
{
    const uint32_t current_track_id = m_path_follower_system->GetCurrentPathEntity(m_entity_id);
    if(current_track_id == m_branch_decided_on_track_id)
        return;

    if(DistanceToTrackEnd() > tweak_values::branch_decision_distance)
        return;

    const bool forward = (m_direction > 0.0f);
    const uint32_t switch_entity_id = m_railway_system->FindBranchingSwitchAhead(m_entity_id, forward);
    if(switch_entity_id == mono::INVALID_ID)
        return;

    m_branch_decided_on_track_id = current_track_id;

    if(mono::Chance(tweak_values::switch_toggle_chance))
        m_railway_system->ToggleSwitch(switch_entity_id);
}

void AITrainController::ToStoppedAtStation()
{
    m_state_timer_s = 0.0f;
    m_last_station_id = m_target_station_id;
}

void AITrainController::StoppedAtStation(const mono::UpdateContext& update_context)
{
    ApplyThrottle(0.0f, update_context.delta_s);

    m_state_timer_s += update_context.delta_s;
    if(m_state_timer_s >= tweak_values::station_wait_s)
        m_states.TransitionTo(States::DRIVING);
}

void AITrainController::ToDeadEnd()
{
    m_state_timer_s = 0.0f;
}

void AITrainController::DeadEnd(const mono::UpdateContext& update_context)
{
    ApplyThrottle(0.0f, update_context.delta_s);

    m_state_timer_s += update_context.delta_s;
    if(m_state_timer_s >= tweak_values::dead_end_wait_s && m_throttle <= 0.0f)
    {
        m_direction = -m_direction;

        // Heading for the other end of the same track now, where there may be a switch to decide on.
        m_branch_decided_on_track_id = mono::INVALID_ID;
        m_states.TransitionTo(States::DRIVING);
    }
}

void AITrainController::ToBlocked()
{
    m_state_timer_s = 0.0f;
}

void AITrainController::Blocked(const mono::UpdateContext& update_context)
{
    ApplyThrottle(0.0f, update_context.delta_s);

    const float distance_to_train = DistanceToTrainAhead();
    const bool still_blocked = (distance_to_train >= 0.0f && distance_to_train < tweak_values::train_keep_distance);

    m_state_timer_s = still_blocked ? 0.0f : (m_state_timer_s + update_context.delta_s);
    if(m_state_timer_s >= tweak_values::blocked_clear_wait_s)
        m_states.TransitionTo(States::DRIVING);
}

void AITrainController::ApplyThrottle(float target_throttle, float delta_s)
{
    const float rate = (target_throttle > m_throttle) ? tweak_values::accelerate_per_s : tweak_values::brake_per_s;
    const float max_change = rate * delta_s;
    m_throttle = std::clamp(target_throttle, m_throttle - max_change, m_throttle + max_change);

    m_path_follower_system->SetThrottle(m_entity_id, m_throttle * m_direction);
}

math::Vector AITrainController::TravelDirection() const
{
    return m_path_follower_system->GetTangent(m_entity_id) * m_direction;
}

float AITrainController::DistanceToTrackEnd() const
{
    const float position = m_path_follower_system->GetCurrentPosition(m_entity_id);
    const float length = m_path_follower_system->GetPathLength(m_entity_id);
    return (m_direction > 0.0f) ? (length - position) : position;
}

bool AITrainController::IsAtTrackEnd() const
{
    return (m_direction > 0.0f) ? m_path_follower_system->IsAtPathEnd(m_entity_id) : m_path_follower_system->IsAtPathStart(m_entity_id);
}

float AITrainController::DistanceToStationAhead(uint32_t& out_station_id) const
{
    const math::Vector train_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector travel_direction = TravelDirection();

    float closest_distance = -1.0f;

    m_railway_system->ForEachStation([&, this](uint32_t station_id) {
        if(station_id == m_last_station_id)
            return;

        const math::Vector to_station = m_transform_system->GetWorldPosition(station_id) - train_position;
        const float along = math::Dot(to_station, travel_direction);
        const float sideways = std::fabs(math::Cross(travel_direction, to_station));
        if(along < 0.0f || sideways > tweak_values::track_width)
            return;

        if(closest_distance < 0.0f || along < closest_distance)
        {
            closest_distance = along;
            out_station_id = station_id;
        }
    });

    return closest_distance;
}

float AITrainController::DistanceToTrainAhead() const
{
    const math::Vector train_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector travel_direction = TravelDirection();
    const uint32_t own_chain_root = m_train_car_system->FindChainRoot(m_entity_id);

    // Measured from the front of this train to the closest end of the other car.
    const float own_half_length = m_train_car_system->GetCouplingDistance(m_entity_id) / 2.0f;

    float closest_distance = -1.0f;

    for(uint32_t car_id : m_train_car_system->GetCarIds())
    {
        if(car_id == m_entity_id || m_train_car_system->FindChainRoot(car_id) == own_chain_root)
            continue;

        const math::Vector to_car = m_transform_system->GetWorldPosition(car_id) - train_position;
        const float along = math::Dot(to_car, travel_direction);
        const float sideways = std::fabs(math::Cross(travel_direction, to_car));
        if(along < 0.0f || sideways > tweak_values::track_width)
            continue;

        const float gap = along - own_half_length - m_train_car_system->GetCouplingDistance(car_id) / 2.0f;
        const float distance = std::max(gap, 0.0f);
        if(closest_distance < 0.0f || distance < closest_distance)
            closest_distance = distance;
    }

    return closest_distance;
}

void AITrainController::DrawDebugInfo(IDebugDrawer* debug_drawer) const
{
    const math::Vector train_position = m_transform_system->GetWorldPosition(m_entity_id);
    const math::Vector travel_direction = TravelDirection();

    debug_drawer->DrawLine(train_position, train_position + travel_direction * tweak_values::brake_distance, 1.0f, mono::Color::CYAN);

    const char* state_string = "";
    switch(m_states.ActiveState())
    {
    case States::DRIVING:            state_string = "Driving";   break;
    case States::STOPPED_AT_STATION: state_string = "Station";   break;
    case States::DEAD_END:           state_string = "Dead end";  break;
    case States::BLOCKED:            state_string = "Blocked";   break;
    }

    char text[128] = {};
    std::snprintf(text, std::size(text), "%s  throttle %.2f %s", state_string, m_throttle, (m_direction > 0.0f) ? ">" : "<");
    debug_drawer->DrawWorldText(text, train_position + math::Vector(0.0f, 0.8f), mono::Color::CYAN);
}

const char* AITrainController::GetDebugCategory() const
{
    return "AI Train";
}
