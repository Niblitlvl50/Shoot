
#include "SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "System/Hash.h"
#include "Util/Random.h"
#include "Util/Algorithm.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

#include <algorithm>
#include <cassert>
#include <limits>

using namespace game;

namespace
{
    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

SpawnSystem::SpawnSystem(uint32_t n, mono::TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system)
    : m_trigger_system(trigger_system)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_spawn_points(n)
    , m_entity_spawn_points(n)
    , m_despawn_entity_triggers(n)
{
    const std::vector<byte> file_data = file::FileReadAll("res/configs/spawn_config.json");
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    for(const auto& spawn_definition : json["spawn_definitions"])
    {
        SpawnDefinition spawn_def;
        spawn_def.value = spawn_definition["value"];
        spawn_def.entity_file = spawn_definition["entity"];
        m_spawn_definitions.push_back(spawn_def);
    }

    const auto sort_by_value = [](const SpawnDefinition& left, const SpawnDefinition& right){
        return left.value < right.value;
    };
    std::sort(m_spawn_definitions.begin(), m_spawn_definitions.end(), sort_by_value);
}

SpawnSystem::SpawnPointComponent* SpawnSystem::AllocateSpawnPoint(uint32_t entity_id)
{
    SpawnPointComponent component = {};
    component.enable_callback_id = NO_CALLBACK_SET;
    component.disable_callback_id = NO_CALLBACK_SET;
    component.counter_ms = 0;
    component.num_spawns = 0;

    return m_spawn_points.Set(entity_id, std::move(component));
}

void SpawnSystem::ReleaseSpawnPoint(uint32_t entity_id)
{
    SpawnPointComponent* spawn_point = m_spawn_points.Get(entity_id);

    if(spawn_point->enable_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(spawn_point->enable_trigger, spawn_point->enable_callback_id, entity_id);

    if(spawn_point->disable_callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(spawn_point->disable_trigger, spawn_point->disable_callback_id, entity_id);

    for(const SpawnIdAndCallback& spawn_id : spawn_point->active_spawns)
        m_entity_manager->RemoveReleaseCallback(spawn_id.spawned_entity_id, spawn_id.callback_id);

    spawn_point->active_spawns.clear();

    const auto remove_if_spawn_id = [entity_id](const SpawnEvent& spawn_event) {
        return (spawn_event.spawner_id == entity_id);
    };
    mono::remove_if(m_spawn_events, remove_if_spawn_id);

    m_spawn_points.Release(entity_id);
}

bool SpawnSystem::IsAllocated(uint32_t entity_id)
{
    return m_spawn_points.IsActive(entity_id);
}

void SpawnSystem::SetSpawnPointData(uint32_t entity_id, const SpawnSystem::SpawnPointComponent& component_data)
{
    SpawnPointComponent* spawn_point = m_spawn_points.Get(entity_id);
    spawn_point->spawn_score = component_data.spawn_score;
    spawn_point->spawn_limit_total = component_data.spawn_limit_total;
    spawn_point->spawn_limit_concurrent = component_data.spawn_limit_concurrent;
    spawn_point->interval_ms = component_data.interval_ms;
    spawn_point->radius = component_data.radius;
    spawn_point->enable_trigger = component_data.enable_trigger;
    spawn_point->disable_trigger = component_data.disable_trigger;
    spawn_point->points = component_data.points;

    if(spawn_point->enable_trigger != 0)
    {
        const mono::TriggerCallback enable_callback = [spawn_point](uint32_t entity_id) {
            spawn_point->active = true;
        };
        spawn_point->enable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point->enable_trigger, enable_callback, entity_id);
    }

    if(spawn_point->disable_trigger != 0)
    {
        const mono::TriggerCallback disable_callback = [spawn_point](uint32_t entity_id) {
            spawn_point->active = false;
        };
        spawn_point->disable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point->disable_trigger, disable_callback, entity_id);
    }

    // If the enable trigger is not set, its enabled from the start.
    spawn_point->active = (spawn_point->enable_trigger == 0);
}

SpawnSystem::EntitySpawnPointComponent* SpawnSystem::AllocateEntitySpawnPoint(uint32_t entity_id)
{
    EntitySpawnPointComponent* allocated_component = m_entity_spawn_points.Set(entity_id, EntitySpawnPointComponent());
    allocated_component->entity_id = entity_id;
    allocated_component->callback_id = NO_CALLBACK_SET;

    return allocated_component;
}

void SpawnSystem::ReleaseEntitySpawnPoint(uint32_t entity_id)
{
    EntitySpawnPointComponent* component = m_entity_spawn_points.Get(entity_id);
    if(component->callback_id != NO_CALLBACK_SET)
    {
        m_trigger_system->RemoveTriggerCallback(component->spawn_trigger, component->callback_id, entity_id);
        component->callback_id = NO_CALLBACK_SET;
    }

    const auto remove_if_spawn_id = [entity_id](const SpawnEvent& spawn_event) {
        return (spawn_event.spawner_id == entity_id);
    };
    mono::remove_if(m_spawn_events, remove_if_spawn_id);

    m_entity_spawn_points.Release(entity_id);
}

void SpawnSystem::SetEntitySpawnPointData(uint32_t entity_id, const std::string& entity_file, float spawn_radius, uint32_t spawn_trigger)
{
    EntitySpawnPointComponent* component = m_entity_spawn_points.Get(entity_id);
    component->entity_file = entity_file;
    component->radius = spawn_radius;
    component->spawn_trigger = spawn_trigger;

    if(component->callback_id != NO_CALLBACK_SET)
        m_trigger_system->RemoveTriggerCallback(component->spawn_trigger, component->callback_id, entity_id);

    if(component->spawn_trigger != 0)
    {
        const mono::TriggerCallback enable_callback = [this, component](uint32_t entity_id) {
            const bool is_added = mono::contains(m_active_entity_spawn_points, component);
            if(!is_added)
                m_active_entity_spawn_points.push_back(component);
        };
        component->callback_id = m_trigger_system->RegisterTriggerCallback(component->spawn_trigger, enable_callback, entity_id);
    }
}

void SpawnSystem::AllocateDespawnTrigger(uint32_t entity_id)
{
    DespawnEntityComponent* component = m_despawn_entity_triggers.Set(entity_id, DespawnEntityComponent());
    component->entity_id = entity_id;
    component->callback_id = NO_CALLBACK_SET;
}

void SpawnSystem::ReleaseDespawnTrigger(uint32_t entity_id)
{
    DespawnEntityComponent* component = m_despawn_entity_triggers.Get(entity_id);
    if(component->callback_id != NO_CALLBACK_SET)
    {
        m_trigger_system->RemoveTriggerCallback(component->despawn_trigger_hash, component->callback_id, entity_id);
        component->callback_id = NO_CALLBACK_SET;
    }

    m_despawn_entity_triggers.Release(entity_id);
}

void SpawnSystem::SetDespawnTriggerData(uint32_t entity_id, uint32_t despawn_trigger_hash)
{
    DespawnEntityComponent* component = m_despawn_entity_triggers.Get(entity_id);
    component->despawn_trigger_hash = despawn_trigger_hash;
    component->entity_id = entity_id;

    if(component->callback_id != NO_CALLBACK_SET)
    {
        m_trigger_system->RemoveTriggerCallback(component->despawn_trigger_hash, component->callback_id, entity_id);
        component->callback_id = NO_CALLBACK_SET;
    }

    if(component->despawn_trigger_hash != 0)
    {
        const mono::TriggerCallback despawn_callback = [component, this](uint32_t trigger_hash) {
            m_entity_manager->ReleaseEntity(component->entity_id);
        };
        component->callback_id = m_trigger_system->RegisterTriggerCallback(component->despawn_trigger_hash, despawn_callback, entity_id);
    }
}

int SpawnSystem::GetActiveSpawns(const SpawnPointComponent* spawn_point)
{
    return spawn_point->active_spawns.size();
}

const std::vector<SpawnSystem::SpawnEvent>& SpawnSystem::GetSpawnEvents() const
{
    return m_spawn_events;
}

uint32_t SpawnSystem::AddGlobalSpawnCallback(const SpawnSystem::SpawnCallback& callback)
{
    uint32_t free_index = std::numeric_limits<uint32_t>::max();

    for(uint32_t index = 0; index < std::size(m_spawn_callbacks); ++index)
    {
        if(m_spawn_callbacks[index].callback == nullptr)
        {
            free_index = index;
            break;
        }
    }

    MONO_ASSERT(free_index != std::numeric_limits<uint32_t>::max());
    m_spawn_callbacks[free_index] = { callback };
    return free_index;
}

void SpawnSystem::RemoveGlobalSpawnCallback(uint32_t callback_id)
{
    m_spawn_callbacks[callback_id].callback = nullptr;
}

const char* SpawnSystem::Name() const
{
    return "spawnsystem";
}

void SpawnSystem::Update(const mono::UpdateContext& update_context)
{
    const auto collect_spawn_points = [&](uint32_t entity_id, SpawnPointComponent& spawn_point) {
        if(!spawn_point.active)
            return;

        if(spawn_point.points.empty())
            return;

        if(spawn_point.spawn_limit_total > 0)
        {
            if(spawn_point.num_spawns >= spawn_point.spawn_limit_total)
                return;
        }

        if(spawn_point.spawn_limit_concurrent > 0)
        {
            if(spawn_point.active_spawns.size() >= size_t(spawn_point.spawn_limit_concurrent))
                return;
        }

        spawn_point.counter_ms += update_context.delta_ms;
        if(spawn_point.counter_ms < spawn_point.interval_ms)
            return;

        spawn_point.counter_ms = 0;
        spawn_point.num_spawns++;

        const float random_length = mono::Random(0.0f, spawn_point.radius);
        const math::Vector random_vector = math::VectorFromAngle(mono::Random(0.0f, math::PI() * 2.0f)) * random_length;

        const int spawn_point_index = mono::RandomInt(0, spawn_point.points.size() -1);
        const math::Vector& local_offset = spawn_point.points[spawn_point_index];

        math::Matrix world_transform = m_transform_system->GetWorld(entity_id);
        math::Translate(world_transform, local_offset + random_vector);

        SpawnEvent spawn_event;
        spawn_event.spawn_score = spawn_point.spawn_score;
        spawn_event.spawner_id = entity_id;
        spawn_event.spawned_entity_id = 0;
        spawn_event.transform = world_transform;
        spawn_event.timestamp_to_spawn = update_context.timestamp + spawn_delay_time_ms;
        m_spawn_events.push_back(spawn_event);
    };
    m_spawn_points.ForEach(collect_spawn_points);

    for(const EntitySpawnPointComponent* spawn_point : m_active_entity_spawn_points)
    {
        const float random_length = mono::Random(0.0f, spawn_point->radius);
        const math::Vector random_vector = math::VectorFromAngle(mono::Random(0.0f, math::PI() * 2.0f)) * random_length;

        math::Matrix world_transform = m_transform_system->GetWorld(spawn_point->entity_id);
        math::Translate(world_transform, random_vector);

        SpawnEvent spawn_event;
        spawn_event.spawner_id = spawn_point->entity_id;
        spawn_event.spawned_entity_id = 0;
        spawn_event.transform = world_transform;
        spawn_event.timestamp_to_spawn = update_context.timestamp + spawn_delay_time_ms;

        spawn_event.entity_file = spawn_point->entity_file;
        spawn_event.spawn_score = 0;

        m_spawn_events.push_back(spawn_event);
    }

    for(SpawnEvent& spawn_event : m_spawn_events)
    {
        const bool time_to_spawn = (spawn_event.timestamp_to_spawn < update_context.timestamp);
        if(!time_to_spawn)
            continue;

        SpawnDefinition spawn_definition;
        spawn_definition.entity_file = spawn_event.entity_file;
        spawn_definition.value = spawn_event.spawn_score;

        if(spawn_definition.entity_file.empty())
        {
            // From spawn_score, lookup some entities to spawn. 

            struct FindByValue
            {
                bool operator() (const SpawnDefinition& spawn_def, int i) const { return spawn_def.value < i; }
                bool operator() (int i, const SpawnDefinition& spawn_def) const { return i < spawn_def.value; }
            };

            const auto pair_it = mono::equal_range(
                m_spawn_definitions.begin(), m_spawn_definitions.end(), spawn_event.spawn_score, FindByValue());

            const uint32_t offset_from_start = std::distance(m_spawn_definitions.begin(), pair_it.first);
            const uint32_t range = std::distance(pair_it.first, pair_it.second);
            const uint32_t spawn_def_index = mono::RandomInt(0, std::max(0, int(range) - 1)) + offset_from_start;

            const uint32_t clamped_spawn_def_index = std::clamp(spawn_def_index, 0u, uint32_t(m_spawn_definitions.size() -1));

            spawn_definition = m_spawn_definitions[clamped_spawn_def_index];
        }

        mono::Entity spawned_entity = m_entity_manager->SpawnEntity(spawn_definition.entity_file.c_str());

        // This spawn might be from a entity spawn point.
        const bool has_spawn_point_component = m_spawn_points.IsActive(spawn_event.spawner_id);
        if(has_spawn_point_component)
        {
            SpawnPointComponent* spawn_component = m_spawn_points.Get(spawn_event.spawner_id);
            if(spawn_component->spawn_limit_concurrent > 0)
            {
                SpawnIdAndCallback spawn_callback_id;
                spawn_callback_id.spawned_entity_id = spawned_entity.id;

                const mono::ReleaseCallback release_callback = [spawn_component](uint32_t entity_id, mono::ReleasePhase phase) {

                    const auto find_spawn_id = [entity_id](const SpawnIdAndCallback& spawn_callback_id) {
                        return spawn_callback_id.spawned_entity_id == entity_id;
                    };
                    mono::remove_if(spawn_component->active_spawns, find_spawn_id);
                };
                spawn_callback_id.callback_id = m_entity_manager->AddReleaseCallback(spawned_entity.id, mono::ReleasePhase::POST_RELEASE, release_callback);

                spawn_component->active_spawns.push_back(spawn_callback_id);
            }
        }

        m_transform_system->SetTransform(spawned_entity.id, spawn_event.transform, mono::TransformState::CLIENT);

        spawn_event.spawned_entity_id = spawned_entity.id;
    }
}

void SpawnSystem::Sync()
{
    m_active_entity_spawn_points.clear();

    const auto remove_if_spawned = [this](const SpawnEvent& spawn_event) {

        const bool time_to_remove = (spawn_event.spawned_entity_id != 0);
        if(time_to_remove)
        {
            for(const SpawnCallbackData& spawn_callback : m_spawn_callbacks)
            {
                if(spawn_callback.callback != nullptr)
                    spawn_callback.callback(spawn_event.spawned_entity_id, spawn_event.spawn_score);
            }
        }
        return time_to_remove;
    };
    mono::remove_if(m_spawn_events, remove_if_spawned);
}

void SpawnSystem::Reset()
{
    Sync();
    m_spawn_events.clear();
}
