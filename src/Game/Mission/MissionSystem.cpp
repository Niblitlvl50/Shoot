
#include "MissionSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "System/File.h"
#include "System/Hash.h"
#include "System/System.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"
#include "Util/Random.h"

#include "nlohmann/json.hpp"

#include <limits>

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

namespace game
{
    static bool operator==(const game::MissionTrackerComponent& left, const game::MissionTrackerComponent& right)
    {
        return left.entity_id == right.entity_id;
    }
}

using namespace game;

MissionSystem::MissionSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, game::TriggerSystem* trigger_system)
    : m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
    , m_point_index(0)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/configs/mission_config.json");
    if(config_file)
    {
        const std::vector<byte>& file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);
        m_spawnable_missions = json["missions"];
    }
}

const char* MissionSystem::Name() const
{
    return "missionsystem";
}

void MissionSystem::Begin()
{
    // Trigger auto trigger missions
    for(const MissionTrackerComponent& mission : m_mission_trackers)
    {
        if(mission.activated_trigger == hash::NO_HASH)
            HandleMissionActivated(mission.entity_id);
    }
}

void MissionSystem::Sync()
{
    m_mission_status_events.clear();
}

void MissionSystem::Update(const mono::UpdateContext& update_context)
{
    for(MissionTrackerComponent& mission_tracker : m_mission_trackers)
    {
        if(mission_tracker.status != MissionStatus::Active || !mission_tracker.time_based)
            continue;

        mission_tracker.time_s -= update_context.delta_s;
 
        if(mission_tracker.time_s <= 0.0f)
        {
            if(mission_tracker.fail_on_timeout)
                HandleMissionFailed(mission_tracker.entity_id);
            else
                HandleMissionCompleted(mission_tracker.entity_id);
        }
    }
}

void MissionSystem::InitializeMissionPositions(const std::vector<uint32_t>& mission_points)
{
    m_mission_points = mission_points;

    mono::UniformRandomBitGenerator random_bit_generator(System::GetMilliseconds());
    std::shuffle(m_mission_points.begin(), m_mission_points.end(), random_bit_generator);
}

void MissionSystem::ActivateMission()
{
    if(m_mission_points.empty())
        return;

    if(m_point_index >= m_mission_points.size())
        return;

    if(m_spawnable_missions.empty())
        return;

    const uint32_t mission_point_id = m_mission_points[m_point_index];
    const math::Vector& world_position = m_transform_system->GetWorldPosition(mission_point_id);

    // Just the first mission for now
    const std::string& selected_mission = m_spawnable_missions.front();
    const std::vector<mono::Entity>& spawned_entities = m_entity_manager->SpawnEntityCollection(selected_mission.c_str());

    // This should position the entities at the mission point.
    for(const mono::Entity& entity : spawned_entities)
    {
        math::Matrix& transform = m_transform_system->GetTransform(entity.id);
        math::Translate(transform, world_position);
    }

    m_point_index++;
}

const std::vector<MissionStatusEvent>& MissionSystem::GetMissionStatusEvents() const
{
    return m_mission_status_events;
}

void MissionSystem::AllocateMission(uint32_t entity_id)
{
    MissionTrackerComponent component;
    component.entity_id = entity_id;

    component.time_based = false;
    component.time_s = 0.0f;
    component.fail_on_timeout = false;

    component.activated_trigger = hash::NO_HASH;
    component.completed_trigger = hash::NO_HASH;
    component.failed_trigger = hash::NO_HASH;

    component.activated_callback_id = NO_CALLBACK_SET;
    component.completed_callback_id = NO_CALLBACK_SET;
    component.failed_callback_id = NO_CALLBACK_SET;

    component.status = MissionStatus::Inactive;

    m_mission_trackers.push_back(component);
}

void MissionSystem::ReleaseMission(uint32_t entity_id)
{
    MissionTrackerComponent* component = GetComponentById(entity_id);
    if(!component)
        return;

    if(component->activated_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->activated_trigger, component->activated_callback_id, entity_id);

    if(component->completed_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->completed_trigger, component->completed_callback_id, entity_id);

    if(component->failed_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->failed_trigger, component->failed_callback_id, entity_id);

    mono::remove(m_mission_trackers, *component);
}

void MissionSystem::SetMissionData(
    uint32_t entity_id,
    const std::string& name,
    const std::string& description,
    bool time_based,
    float time_s,
    bool fail_on_timeout,
    uint32_t activated_trigger_hash,
    uint32_t completed_trigger_hash,
    uint32_t failed_trigger_hash)
{
    MissionTrackerComponent* component = GetComponentById(entity_id);
    if(!component)
        return;

    component->name = name;
    component->description = description;
    component->time_based = time_based;
    component->time_s = time_s;
    component->fail_on_timeout = fail_on_timeout;
    component->activated_trigger = activated_trigger_hash;
    component->completed_trigger = completed_trigger_hash;
    component->failed_trigger = failed_trigger_hash;

    if(component->activated_trigger != hash::NO_HASH)
    {
        const game::TriggerCallback activated_callback = [this, entity_id](uint32_t trigger_id) {
            HandleMissionActivated(entity_id);
        };
        component->activated_callback_id = m_trigger_system->RegisterTriggerCallback(component->activated_trigger, activated_callback, entity_id);
    }

    if(component->completed_trigger != hash::NO_HASH)
    {
        const game::TriggerCallback completed_callback = [this, entity_id](uint32_t trigger_id) {
            HandleMissionCompleted(entity_id);
        };
        component->completed_callback_id = m_trigger_system->RegisterTriggerCallback(component->completed_trigger, completed_callback, entity_id);
    }

    if(component->failed_trigger != hash::NO_HASH)
    {
        const game::TriggerCallback failed_callback = [this, entity_id](uint32_t trigger_id) {
            HandleMissionFailed(entity_id);
        };
        component->failed_callback_id = m_trigger_system->RegisterTriggerCallback(component->failed_trigger, failed_callback, entity_id);
    }
}

const MissionTrackerComponent* MissionSystem::GetComponentById(uint32_t entity_id) const
{
    return const_cast<MissionSystem*>(this)->GetComponentById(entity_id);
}

MissionTrackerComponent* MissionSystem::GetComponentById(uint32_t entity_id)
{
    const auto find_by_id = [entity_id](const MissionTrackerComponent& mission_tracker) {
        return mission_tracker.entity_id == entity_id;
    };
    return mono::find_if(m_mission_trackers, find_by_id);
}

void MissionSystem::HandleMissionActivated(uint32_t entity_id)
{
    MissionTrackerComponent* component = GetComponentById(entity_id);
    if(component->status >= MissionStatus::Active)
        return;

    component->status = MissionStatus::Active;
    m_mission_status_events.push_back({ entity_id, component->status });

    System::Log("MissionSystem|Mission Activated! %s (%s)", component->name.c_str(), component->description.c_str());
}

void MissionSystem::HandleMissionCompleted(uint32_t entity_id)
{
    MissionTrackerComponent* component = GetComponentById(entity_id);
    if(component->status >= MissionStatus::Completed)
        return;

    component->status = MissionStatus::Completed;
    m_mission_status_events.push_back({ entity_id, component->status });

    System::Log("MissionSystem|Mission Completed! %s (%s)", component->name.c_str(), component->description.c_str());
}

void MissionSystem::HandleMissionFailed(uint32_t entity_id)
{
    MissionTrackerComponent* component = GetComponentById(entity_id);
    if(component->status >= MissionStatus::Failed)
        return;

    component->status = MissionStatus::Failed;
    m_mission_status_events.push_back({ entity_id, component->status });

    System::Log("MissionSystem|Mission Failed! %s (%s)", component->name.c_str(), component->description.c_str());
}
