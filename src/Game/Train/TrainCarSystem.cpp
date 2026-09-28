
#include "TrainCarSystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Behaviour/PathFollowerSystem.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Vector.h"
#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

using namespace game;

namespace tweak_values
{
    // Throttle (per meter of coupling-gap error) used to close the gap to the leader.
    constexpr float coupling_gain = 1.5f;

    // Distance between two free coupling points within which their cars auto-couple.
    constexpr float coupling_trigger_distance = 0.5f;

    // Grace period after decoupling before a car becomes eligible for auto-coupling again,
    // so it doesn't immediately re-attach to whatever it just detached from.
    constexpr float decouple_cooldown_s = 1.5f;
}

namespace
{
    // Leader's actual signed speed along its own path (meters/second), regardless of
    // whether it's manually throttled or advancing automatically. GetThrottle alone isn't
    // enough - it's a normalized [-1,1] value, not an actual speed.
    float GetLeaderArcVelocity(const game::PathFollowerSystem* path_follower_system, uint32_t leader_entity_id)
    {
        const float leader_speed = path_follower_system->GetSpeed(leader_entity_id);
        const float leader_direction = path_follower_system->IsManualControl(leader_entity_id)
            ? path_follower_system->GetThrottle(leader_entity_id)
            : path_follower_system->GetDirection(leader_entity_id);

        return leader_direction * leader_speed;
    }
}

TrainCarSystem::TrainCarSystem(mono::SystemContext* system_context)
    : m_system_context(system_context)
{
    m_couple_sound = audio::CreateSound(
        "res/sound/train/train_car_slam_couple.wav", audio::SoundPlayback::ONCE, audio::SoundSpatiality::NONE);
    m_decouple_sound = audio::CreateSound(
        "res/sound/train/train_car_slam_decouple.wav", audio::SoundPlayback::ONCE, audio::SoundSpatiality::NONE);
}

TrainCarComponent* TrainCarSystem::AllocateTrainCar(uint32_t entity_id)
{
    // The entity's path_follower component (a declared dependency) owns allocating and
    // configuring the actual PathFollowerComponent - manual_control and apply_rotation
    // must be set on it directly in the editor.
    return &m_cars[entity_id];
}

void TrainCarSystem::ReleaseTrainCar(uint32_t entity_id)
{
    // Anything coupled to this car loses its leader rather than following a dead entity.
    for(auto& entity_car_pair : m_cars)
    {
        if(entity_car_pair.second.leader_entity_id == entity_id)
            entity_car_pair.second.leader_entity_id = mono::INVALID_ID;
    }

    m_cars.erase(entity_id);
}

void TrainCarSystem::SetTrainCarData(uint32_t entity_id, float coupling_distance, bool is_locomotive)
{
    const auto it = m_cars.find(entity_id);
    if(it == m_cars.end())
        return;

    it->second.coupling_distance = coupling_distance;
    it->second.is_locomotive = is_locomotive;
}

void TrainCarSystem::SetLeader(uint32_t entity_id, uint32_t leader_entity_id)
{
    const auto it = m_cars.find(entity_id);
    if(it == m_cars.end())
        return;

    it->second.leader_entity_id = leader_entity_id;

    if(leader_entity_id != mono::INVALID_ID)
        m_couple_sound->Play();
}

void TrainCarSystem::Decouple(uint32_t leader_entity_id)
{
    bool decoupled_any = false;

    game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    for(auto& entity_car_pair : m_cars)
    {
        TrainCarComponent& car = entity_car_pair.second;
        if(car.leader_entity_id != leader_entity_id)
            continue;

        path_follower_system->SetThrottle(entity_car_pair.first, 0.0f);

        car.leader_entity_id = mono::INVALID_ID;
        car.decouple_cooldown_s = tweak_values::decouple_cooldown_s;
        decoupled_any = true;
    }

    if(decoupled_any)
        m_decouple_sound->Play();
}

const char* TrainCarSystem::Name() const
{
    return "traincarsystem";
}

float TrainCarSystem::GetCouplingDistance(uint32_t entity_id) const
{
    const auto it = m_cars.find(entity_id);
    return (it != m_cars.end()) ? it->second.coupling_distance : TrainCarComponent().coupling_distance;
}

TrainCarSystem::CouplingPoint TrainCarSystem::GetCouplingPoint(uint32_t entity_id, float side) const
{
    const mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    const game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    const math::Vector center = transform_system->GetWorldPosition(entity_id);
    const math::Vector tangent = path_follower_system->GetTangent(entity_id);

    CouplingPoint coupling_point;
    coupling_point.position = center + tangent * (side * GetCouplingDistance(entity_id) * 0.5f);
    coupling_point.is_free = true;

    // An end is taken if a neighbour (the leader, or a car led by this one) sits on that
    // side. Worked out geometrically rather than stored, so it stays right when a switch
    // hand-off rebakes the car's track in the opposite direction.
    const auto is_on_this_side = [&](uint32_t neighbour_id) {
        const math::Vector to_neighbour = transform_system->GetWorldPosition(neighbour_id) - center;
        return math::Dot(to_neighbour, tangent) * side > 0.0f;
    };

    const auto it = m_cars.find(entity_id);
    if(it != m_cars.end() && it->second.leader_entity_id != mono::INVALID_ID && is_on_this_side(it->second.leader_entity_id))
        coupling_point.is_free = false;

    for(const auto& entity_car_pair : m_cars)
    {
        if(entity_car_pair.second.leader_entity_id == entity_id && is_on_this_side(entity_car_pair.first))
            coupling_point.is_free = false;
    }

    return coupling_point;
}

uint32_t TrainCarSystem::FindChainRoot(uint32_t entity_id) const
{
    uint32_t current = entity_id;

    // Bounded by the car count as a guard against a malformed (cyclic) chain.
    for(size_t step = 0; step <= m_cars.size(); ++step)
    {
        const auto it = m_cars.find(current);
        if(it == m_cars.end() || it->second.leader_entity_id == mono::INVALID_ID)
            return current;

        current = it->second.leader_entity_id;
    }

    return current;
}

void TrainCarSystem::MakeChainRoot(uint32_t entity_id)
{
    uint32_t previous = mono::INVALID_ID;
    uint32_t current = entity_id;

    for(size_t step = 0; step <= m_cars.size() && current != mono::INVALID_ID; ++step)
    {
        const auto it = m_cars.find(current);
        if(it == m_cars.end())
            break;

        const uint32_t next = it->second.leader_entity_id;
        it->second.leader_entity_id = previous;
        previous = current;
        current = next;
    }
}

void TrainCarSystem::Sync()
{
    if(m_cars.empty())
        return;

    mono::IEntityManager* entity_manager = m_system_context->GetSystem<mono::IEntityManager>();
    game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    for(const auto& entity_car_pair : m_cars)
    {
        const uint32_t car_entity_id = entity_car_pair.first;
        const TrainCarComponent& car = entity_car_pair.second;

        // A car already on a track stays on it. Only one that was placed without a path
        // is put on its leader's track, at the point nearest to where it already is.
        if(car.leader_entity_id == mono::INVALID_ID || path_follower_system->GetCurrentPathEntity(car_entity_id) != mono::INVALID_ID)
            continue;

        const uint32_t leader_track_id = path_follower_system->GetCurrentPathEntity(car.leader_entity_id);
        if(leader_track_id == mono::INVALID_ID)
            continue;

        path_follower_system->SetPathReferenceAtCurrentPosition(car_entity_id, entity_manager->GetEntityUuid(leader_track_id));
    }
}

void TrainCarSystem::TryAutoCouple()
{
    constexpr float sides[] = { 1.0f, -1.0f };

    for(const auto& entity_car_pair : m_cars)
    {
        const uint32_t car_entity_id = entity_car_pair.first;

        // Only a car whose chain isn't driven by a locomotive can become a follower.
        const uint32_t chain_root_id = FindChainRoot(car_entity_id);
        const TrainCarComponent& chain_root = m_cars.at(chain_root_id);
        if(chain_root.is_locomotive || chain_root.decouple_cooldown_s > 0.0f)
            continue;

        uint32_t best_anchor_id = mono::INVALID_ID;
        float best_distance = tweak_values::coupling_trigger_distance;

        for(float car_side : sides)
        {
            const CouplingPoint car_point = GetCouplingPoint(car_entity_id, car_side);
            if(!car_point.is_free)
                continue;

            for(const auto& other_car_pair : m_cars)
            {
                const uint32_t other_entity_id = other_car_pair.first;
                if(FindChainRoot(other_entity_id) == chain_root_id)
                    continue;

                for(float other_side : sides)
                {
                    const CouplingPoint other_point = GetCouplingPoint(other_entity_id, other_side);
                    if(!other_point.is_free)
                        continue;

                    const float distance = math::DistanceBetween(car_point.position, other_point.position);
                    if(distance < best_distance)
                    {
                        best_distance = distance;
                        best_anchor_id = other_entity_id;
                    }
                }
            }
        }

        if(best_anchor_id == mono::INVALID_ID)
            continue;

        // This car becomes the head of its own chain so it can take the anchor as leader;
        // the rest of its chain then follows it, one car after the other.
        MakeChainRoot(car_entity_id);
        SetLeader(car_entity_id, best_anchor_id);
    }
}

void TrainCarSystem::Update(const mono::UpdateContext& update_context)
{
    if(m_cars.empty())
        return;

    for(auto& entity_car_pair : m_cars)
    {
        if(entity_car_pair.second.decouple_cooldown_s > 0.0f)
            entity_car_pair.second.decouple_cooldown_s -= update_context.delta_s;
    }

    TryAutoCouple();

    const mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    for(const auto& entity_car_pair : m_cars)
    {
        const uint32_t car_entity_id = entity_car_pair.first;
        const TrainCarComponent& car = entity_car_pair.second;

        if(car.leader_entity_id == mono::INVALID_ID || path_follower_system->GetCurrentPathEntity(car_entity_id) == mono::INVALID_ID)
            continue;

        // Everything is measured in world space and projected onto this car's own track
        // direction, so it doesn't matter which track the leader is on or which way either
        // track's points were baked.
        const math::Vector car_tangent = path_follower_system->GetTangent(car_entity_id);
        const math::Vector to_leader =
            transform_system->GetWorldPosition(car.leader_entity_id) - transform_system->GetWorldPosition(car_entity_id);
        const float toward_leader = (math::Dot(to_leader, car_tangent) >= 0.0f) ? 1.0f : -1.0f;

        // Match the leader's speed...
        const math::Vector leader_velocity =
            path_follower_system->GetTangent(car.leader_entity_id) * GetLeaderArcVelocity(path_follower_system, car.leader_entity_id);
        float desired_arc_velocity = math::Dot(leader_velocity, car_tangent);

        // ...and close any drift in the gap between them.
        const float target_gap = (car.coupling_distance + GetCouplingDistance(car.leader_entity_id)) * 0.5f;
        const float gap_error = math::Length(to_leader) - target_gap;
        desired_arc_velocity += toward_leader * gap_error * tweak_values::coupling_gain;

        const float car_speed = path_follower_system->GetSpeed(car_entity_id);
        const float throttle = (car_speed > 0.0f) ? std::clamp(desired_arc_velocity / car_speed, -1.0f, 1.0f) : 0.0f;
        path_follower_system->SetThrottle(car_entity_id, throttle);
    }

    if(game::g_draw_train_car_debug)
        DrawDebugInfo();
}

void TrainCarSystem::DrawDebugInfo() const
{
    const mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    const game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    constexpr float marker_radius = 0.2f;
    constexpr float sides[] = { 1.0f, -1.0f };

    for(const auto& entity_car_pair : m_cars)
    {
        const uint32_t car_entity_id = entity_car_pair.first;
        const TrainCarComponent& car = entity_car_pair.second;

        const math::Vector car_position = transform_system->GetWorldPosition(car_entity_id);
        const bool has_track = (path_follower_system->GetCurrentPathEntity(car_entity_id) != mono::INVALID_ID);

        mono::Color::RGBA marker_color;
        const char* state_text = nullptr;

        if(car.is_locomotive)
        {
            marker_color = mono::Color::GOLDEN_YELLOW;
            state_text = "locomotive";
        }
        else if(!has_track)
        {
            marker_color = mono::Color::MAGENTA;
            state_text = "no track";
        }
        else if(car.decouple_cooldown_s > 0.0f)
        {
            marker_color = mono::Color::GRAY;
            state_text = "cooldown";
        }
        else if(car.leader_entity_id == mono::INVALID_ID)
        {
            marker_color = mono::Color::RED;
            state_text = "no leader";
        }
        else
        {
            marker_color = mono::Color::GREEN;
            state_text = "coupled";
        }

        game::g_debug_drawer->DrawCircle(car_position, marker_radius, marker_color);

        // Coupling points: a free one also shows the range it auto-couples within.
        for(float side : sides)
        {
            const CouplingPoint coupling_point = GetCouplingPoint(car_entity_id, side);
            if(coupling_point.is_free)
            {
                game::g_debug_drawer->DrawPoint(coupling_point.position, 8.0f, mono::Color::CYAN);
                game::g_debug_drawer->DrawCircle(
                    coupling_point.position, tweak_values::coupling_trigger_distance, mono::Color::RGBA(0.0f, 1.0f, 1.0f, 0.25f));
            }
            else
            {
                game::g_debug_drawer->DrawPoint(coupling_point.position, 8.0f, mono::Color::ORANGE);
            }
        }

        char label[256] = {};
        std::snprintf(
            label,
            std::size(label),
            "train_car[%u] %s  root:%u  throttle:%.2f",
            car_entity_id,
            state_text,
            FindChainRoot(car_entity_id),
            path_follower_system->GetThrottle(car_entity_id));
        game::g_debug_drawer->DrawWorldText(label, car_position + math::Vector(0.0f, 0.5f), marker_color);

        if(car.leader_entity_id != mono::INVALID_ID)
        {
            const math::Vector leader_position = transform_system->GetWorldPosition(car.leader_entity_id);
            game::g_debug_drawer->DrawLine(car_position, leader_position, 2.0f, marker_color);
        }
    }
}
