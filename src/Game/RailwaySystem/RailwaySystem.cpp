
#include "RailwaySystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Behaviour/PathFollowerSystem.h"
#include "Math/Vector.h"
#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

using namespace game;

namespace
{
    constexpr float snap_tolerance = 1.0f;
}

RailwaySystem::RailwaySystem(mono::SystemContext* system_context)
    : m_system_context(system_context)
{ }

RailwaySwitchComponent* RailwaySystem::AllocateSwitch(uint32_t entity_id)
{
    return &m_switches[entity_id];
}

void RailwaySystem::ReleaseSwitch(uint32_t entity_id)
{
    const auto it = m_switches.find(entity_id);
    if(it != m_switches.end() && it->second.trigger_callback_id != mono::INVALID_ID)
    {
        mono::TriggerSystem* trigger_system = m_system_context->GetSystem<mono::TriggerSystem>();
        trigger_system->RemoveTriggerCallback(it->second.trigger_hash, it->second.trigger_callback_id, entity_id);
    }

    m_switches.erase(entity_id);
}

void RailwaySystem::SetSwitchData(
    uint32_t entity_id,
    uint32_t trunk_track_reference,
    uint32_t primary_track_reference,
    uint32_t alt_track_reference,
    bool use_alt_branch,
    uint32_t trigger_hash,
    uint32_t primary_trigger_hash,
    uint32_t alt_trigger_hash)
{
    const auto it = m_switches.find(entity_id);
    if(it == m_switches.end())
        return;

    RailwaySwitchComponent& railway_switch = it->second;

    mono::IEntityManager* entity_manager = m_system_context->GetSystem<mono::IEntityManager>();
    railway_switch.trunk_track_entity_id = entity_manager->GetEntityIdFromUuid(trunk_track_reference);
    railway_switch.primary_track_entity_id = entity_manager->GetEntityIdFromUuid(primary_track_reference);
    railway_switch.alt_track_entity_id = entity_manager->GetEntityIdFromUuid(alt_track_reference);
    railway_switch.use_alt_branch = use_alt_branch;
    railway_switch.primary_trigger_hash = primary_trigger_hash;
    railway_switch.alt_trigger_hash = alt_trigger_hash;

    mono::TriggerSystem* trigger_system = m_system_context->GetSystem<mono::TriggerSystem>();
    if(railway_switch.trigger_callback_id != mono::INVALID_ID)
        trigger_system->RemoveTriggerCallback(railway_switch.trigger_hash, railway_switch.trigger_callback_id, entity_id);

    railway_switch.trigger_hash = trigger_hash;

    const mono::TriggerCallback toggle_callback = [this, entity_id](uint32_t trigger_id) {
        ToggleSwitch(entity_id);
    };
    railway_switch.trigger_callback_id = trigger_system->RegisterTriggerCallback(trigger_hash, toggle_callback, entity_id);
}

void RailwaySystem::ToggleSwitch(uint32_t switch_entity_id)
{
    const auto it = m_switches.find(switch_entity_id);
    if(it == m_switches.end())
        return;

    RailwaySwitchComponent& railway_switch = it->second;
    railway_switch.use_alt_branch = !railway_switch.use_alt_branch;

    mono::TriggerSystem* trigger_system = m_system_context->GetSystem<mono::TriggerSystem>();
    trigger_system->EmitTrigger(railway_switch.use_alt_branch ? railway_switch.alt_trigger_hash : railway_switch.primary_trigger_hash);
}

RailwayStationComponent* RailwaySystem::AllocateStation(uint32_t entity_id)
{
    return &m_stations[entity_id];
}

void RailwaySystem::ReleaseStation(uint32_t entity_id)
{
    m_stations.erase(entity_id);
}

void RailwaySystem::SetStationData(uint32_t entity_id, const std::string& name)
{
    const auto it = m_stations.find(entity_id);
    if(it != m_stations.end())
        it->second.name = name;
}

const char* RailwaySystem::Name() const
{
    return "railwaysystem";
}

void RailwaySystem::Update(const mono::UpdateContext& update_context)
{
    game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();
    path_follower_system->ForEach([this](uint32_t entity_id) {
        TryHandOff(entity_id);
    });

    if(game::g_draw_railway_debug)
        DrawDebugInfo();
}

void RailwaySystem::DrawDebugInfo() const
{
    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();

    constexpr float switch_radius = 0.5f;
    constexpr mono::Color::RGBA connection_color(1.0f, 1.0f, 1.0f, 0.3f);

    for(const auto& switch_pair : m_switches)
    {
        const uint32_t switch_entity_id = switch_pair.first;
        const RailwaySwitchComponent& railway_switch = switch_pair.second;

        const math::Vector switch_position = transform_system->GetWorldPosition(switch_entity_id);
        const mono::Color::RGBA active_color = railway_switch.use_alt_branch ? mono::Color::CYAN : mono::Color::ORANGE;

        game::g_debug_drawer->DrawCircle(switch_position, switch_radius, active_color);
        game::g_debug_drawer->DrawWorldText(
            railway_switch.use_alt_branch ? "Switch (Alt)" : "Switch (Primary)", switch_position, active_color);

        if(railway_switch.trunk_track_entity_id != mono::INVALID_ID)
        {
            const math::Vector trunk_position = transform_system->GetWorldPosition(railway_switch.trunk_track_entity_id);
            game::g_debug_drawer->DrawLine(switch_position, trunk_position, 1.0f, connection_color);
        }

        if(railway_switch.primary_track_entity_id != mono::INVALID_ID)
        {
            const math::Vector primary_position = transform_system->GetWorldPosition(railway_switch.primary_track_entity_id);
            const bool is_active = !railway_switch.use_alt_branch;
            game::g_debug_drawer->DrawLine(
                switch_position, primary_position, is_active ? 2.0f : 1.0f, is_active ? mono::Color::ORANGE : connection_color);
        }

        if(railway_switch.alt_track_entity_id != mono::INVALID_ID)
        {
            const math::Vector alt_position = transform_system->GetWorldPosition(railway_switch.alt_track_entity_id);
            const bool is_active = railway_switch.use_alt_branch;
            game::g_debug_drawer->DrawLine(
                switch_position, alt_position, is_active ? 2.0f : 1.0f, is_active ? mono::Color::CYAN : connection_color);
        }
    }

    for(const auto& station_pair : m_stations)
    {
        const uint32_t station_entity_id = station_pair.first;
        const RailwayStationComponent& station = station_pair.second;

        const math::Vector station_position = transform_system->GetWorldPosition(station_entity_id);
        game::g_debug_drawer->DrawPoint(station_position, 8.0f, mono::Color::GOLDEN_YELLOW);

        const char* label = station.name.empty() ? "Station" : station.name.c_str();
        game::g_debug_drawer->DrawWorldText(label, station_position, mono::Color::GOLDEN_YELLOW);
    }
}

void RailwaySystem::TryHandOff(uint32_t train_entity_id)
{
    game::PathFollowerSystem* path_follower_system = m_system_context->GetSystem<game::PathFollowerSystem>();

    const uint32_t current_track_id = path_follower_system->GetCurrentPathEntity(train_entity_id);
    if(current_track_id == mono::INVALID_ID)
        return;

    // Only hand off while actively being driven into the end of the track - never on the
    // resting frame right after a hand-off, which would otherwise bounce straight back.
    const float throttle = path_follower_system->GetThrottle(train_entity_id);
    const bool pushing_forward = (throttle > 0.0f) && path_follower_system->IsAtPathEnd(train_entity_id);
    const bool pushing_backward = (throttle < 0.0f) && path_follower_system->IsAtPathStart(train_entity_id);
    if(!pushing_forward && !pushing_backward)
        return;

    const std::vector<math::Vector>* path_points = path_follower_system->GetPathPoints(train_entity_id);
    if(!path_points || path_points->empty())
        return;

    const math::Vector& endpoint_position = pushing_forward ? path_points->back() : path_points->front();

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();

    for(const auto& switch_pair : m_switches)
    {
        const uint32_t switch_entity_id = switch_pair.first;
        const RailwaySwitchComponent& railway_switch = switch_pair.second;

        const math::Vector switch_position = transform_system->GetWorldPosition(switch_entity_id);
        if(math::DistanceBetween(switch_position, endpoint_position) > snap_tolerance)
            continue;

        uint32_t next_track_id = mono::INVALID_ID;

        if(current_track_id == railway_switch.trunk_track_entity_id)
        {
            next_track_id = railway_switch.use_alt_branch
                ? railway_switch.alt_track_entity_id
                : railway_switch.primary_track_entity_id;
        }
        else if(current_track_id == railway_switch.primary_track_entity_id || current_track_id == railway_switch.alt_track_entity_id)
        {
            next_track_id = railway_switch.trunk_track_entity_id;
        }

        if(next_track_id == mono::INVALID_ID || next_track_id == current_track_id)
            continue;

        if(path_follower_system->SwitchToPathEntity(train_entity_id, next_track_id, endpoint_position, pushing_forward))
            return;
    }
}
