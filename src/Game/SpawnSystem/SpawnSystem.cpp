
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

using namespace game;

SpawnSystem::SpawnSystem(uint32_t n, TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system)
    : m_trigger_system(trigger_system)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
    , m_spawn_points(n)
    , m_entity_spawn_points(n)
{
    file::FilePtr config_file = file::OpenAsciiFile("res/spawn_config.json");
    if(config_file)
    {
        const std::vector<byte> file_data = file::FileRead(config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        for(const auto& spawn_props : json["spawn_definitions"])
        {
            SpawnDefinition spawn_def;
            spawn_def.value = spawn_props["value"];
            spawn_def.entity_file = spawn_props["entity"];
            m_spawn_definitions.push_back(spawn_def);
        }

        const auto sort_by_value = [](const SpawnDefinition& left, const SpawnDefinition& right){
            return left.value < right.value;
        };
        std::sort(m_spawn_definitions.begin(), m_spawn_definitions.end(), sort_by_value);
    }
}

SpawnSystem::SpawnPointComponent* SpawnSystem::AllocateSpawnPoint(uint32_t entity_id)
{
    SpawnPointComponent component = {};
    return m_spawn_points.Set(entity_id, std::move(component));

    //SpawnPointComponent& spawn_point = m_spawn_points[entity_id];
    //std::memset(&spawn_point, 0, sizeof(SpawnPointComponent));

    //return &spawn_point;
}

void SpawnSystem::ReleaseSpawnPoint(uint32_t entity_id)
{
    SpawnPointComponent* spawn_point = m_spawn_points.Get(entity_id);
    if(spawn_point->enable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point->enable_trigger, spawn_point->enable_callback_id, entity_id);

    if(spawn_point->disable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point->disable_trigger, spawn_point->disable_callback_id, entity_id);

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
    spawn_point->radius = component_data.radius;
    spawn_point->interval = component_data.interval;
    spawn_point->properties = component_data.properties;
    spawn_point->enable_trigger = component_data.enable_trigger;
    spawn_point->disable_trigger = component_data.disable_trigger;

    if(spawn_point->enable_trigger != 0)
    {
        const game::TriggerCallback enable_callback = [spawn_point](uint32_t entity_id) {
            spawn_point->active = true;
        };
        spawn_point->enable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point->enable_trigger, enable_callback, entity_id);
    }

    if(spawn_point->disable_trigger != 0)
    {
        const game::TriggerCallback disable_callback = [spawn_point](uint32_t entity_id) {
            spawn_point->active = false;
        };
        spawn_point->disable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point->disable_trigger, disable_callback, entity_id);
    }

    spawn_point->active = false;
}

SpawnSystem::EntitySpawnPointComponent* SpawnSystem::AllocateEntitySpawnPoint(uint32_t entity_id)
{
    EntitySpawnPointComponent* allocated_component = m_entity_spawn_points.Set(entity_id, EntitySpawnPointComponent());
    allocated_component->entity_id = entity_id;

    return allocated_component;
}

void SpawnSystem::ReleaseEntitySpawnPoint(uint32_t entity_id)
{
    EntitySpawnPointComponent* component = m_entity_spawn_points.Get(entity_id);
    if(component->callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(component->spawn_trigger, component->callback_id, entity_id);

    m_entity_spawn_points.Release(entity_id);
}

void SpawnSystem::SetEntitySpawnPointData(uint32_t entity_id, const std::string& entity_file, float spawn_radius, uint32_t spawn_trigger)
{
    EntitySpawnPointComponent* component = m_entity_spawn_points.Get(entity_id);
    component->entity_file = entity_file;
    component->radius = spawn_radius;
    component->spawn_trigger = spawn_trigger;

    if(component->callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(component->spawn_trigger, component->callback_id, entity_id);

    if(component->spawn_trigger != 0)
    {
        const game::TriggerCallback enable_callback = [this, component](uint32_t entity_id) {
            printf("spawn spawn!\n");

            const bool is_added = mono::contains(m_active_entity_spawn_points, component);
            if(!is_added)
                m_active_entity_spawn_points.push_back(component);
        };
        component->callback_id = m_trigger_system->RegisterTriggerCallback(component->spawn_trigger, enable_callback, entity_id);
    }
}

const std::vector<SpawnSystem::SpawnEvent>& SpawnSystem::GetSpawnEvents() const
{
    return m_spawn_events;
}

uint32_t SpawnSystem::Id() const
{
    return hash::Hash(Name());
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

        spawn_point.counter += update_context.delta_ms;
        if(spawn_point.counter < spawn_point.interval)
            return;

        const float random_length = mono::Random(0.0f, spawn_point.radius);
        const math::Vector random_vector = math::VectorFromAngle(mono::Random(0.0f, math::PI() * 2.0f)) * random_length;

        math::Matrix world_transform = m_transform_system->GetWorld(entity_id);
        math::Translate(world_transform, random_vector);

        SpawnEvent spawn_event;
        spawn_event.spawn_score = spawn_point.spawn_score;
        spawn_event.spawner_id = entity_id;
        spawn_event.spawned_entity_id = 0;
        spawn_event.transform = world_transform;
        spawn_event.timestamp_to_spawn = update_context.timestamp + spawn_delay_time_ms;
        m_spawn_events.push_back(spawn_event);

        spawn_point.counter = 0;
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
            //spawn_point.spawn_score;

            struct FindByValue
            {
                bool operator() (const SpawnDefinition& spawn_def, int i) const { return spawn_def.value < i; }
                bool operator() (int i, const SpawnDefinition& spawn_def) const { return i < spawn_def.value; }
            };

            const auto pair_it = mono::equal_range(m_spawn_definitions.begin(), m_spawn_definitions.end(), 1, FindByValue());

            const uint32_t offset_from_start = std::distance(m_spawn_definitions.begin(), pair_it.first);
            const uint32_t range = std::distance(pair_it.first, pair_it.second);
            const uint32_t spawn_def_index = mono::RandomInt(0, range - 1) + offset_from_start;

            spawn_definition = m_spawn_definitions[spawn_def_index];
        }

        mono::Entity spawned_entity = m_entity_manager->CreateEntity(spawn_definition.entity_file.c_str());

        m_transform_system->SetTransform(spawned_entity.id, spawn_event.transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

        spawn_event.spawned_entity_id = spawned_entity.id;
    }
}

void SpawnSystem::Sync()
{
    m_active_entity_spawn_points.clear();

    const auto remove_if_spawned = [](const SpawnEvent& spawn_event) {
        return (spawn_event.spawned_entity_id != 0);
    };
    mono::remove_if(m_spawn_events, remove_if_spawned);
}
