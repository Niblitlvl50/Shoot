
#include "SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "Util/Hash.h"
#include "Util/Random.h"
#include "Util/Algorithm.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

#include <cassert>

using namespace game;

SpawnSystem::SpawnSystem(uint32_t n, TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system)
    : m_trigger_system(trigger_system)
    , m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
{
    m_spawn_points.resize(n);
    m_spawn_points_internal.resize(n);
    m_alive.resize(n, false);

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
    }
}

SpawnSystem::SpawnPointComponent* SpawnSystem::AllocateSpawnPoint(uint32_t entity_id)
{
    assert(!m_alive[entity_id]);
    m_alive[entity_id] = true;

    SpawnPointComponent& spawn_point = m_spawn_points[entity_id];
    std::memset(&spawn_point, 0, sizeof(SpawnPointComponent));

    SpawnPointInternalData& internal_data = m_spawn_points_internal[entity_id];
    std::memset(&internal_data, 0, sizeof(SpawnPointInternalData));

    return &spawn_point;
}

void SpawnSystem::ReleaseSpawnPoint(uint32_t entity_id)
{
    m_alive[entity_id] = false;

    SpawnPointComponent& spawn_point = m_spawn_points[entity_id];
    SpawnPointInternalData& internal_data = m_spawn_points_internal[entity_id];

    if(internal_data.enable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point.enable_trigger, internal_data.enable_callback_id, entity_id);

    if(internal_data.disable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point.disable_trigger, internal_data.disable_callback_id, entity_id);
}

bool SpawnSystem::IsAllocated(uint32_t entity_id)
{
    return m_alive[entity_id];
}

void SpawnSystem::SetSpawnPointData(uint32_t entity_id, const SpawnSystem::SpawnPointComponent& component_data)
{
    assert(m_alive[sprite_id]);
    assert(component_data.interval > spawn_delay_frames);

    SpawnPointComponent& spawn_point = m_spawn_points[entity_id];
    spawn_point = component_data;

    SpawnPointInternalData& internal_data = m_spawn_points_internal[entity_id];

    if(spawn_point.enable_trigger != 0)
    {
        const game::TriggerCallback enable_callback = [&internal_data](uint32_t entity_id) {
            internal_data.active = true;
        };
        internal_data.enable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point.enable_trigger, enable_callback, entity_id);
    }

    if(spawn_point.disable_trigger != 0)
    {
        const game::TriggerCallback disable_callback = [&internal_data](uint32_t entity_id) {
            internal_data.active = false;
        };
        internal_data.disable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point.disable_trigger, disable_callback, entity_id);
    }

    internal_data.active = false;
}

const std::vector<SpawnSystem::SpawnEvent>& SpawnSystem::GetSpawnEvents() const
{
    return m_spawn_events;
}

uint32_t SpawnSystem::Id() const
{
    return mono::Hash(Name());
}

const char* SpawnSystem::Name() const
{
    return "spawnsystem";
}

void SpawnSystem::Update(const mono::UpdateContext& update_context)
{
    for(uint32_t index = 0; index < m_alive.size(); ++index)
    {
        if(!m_alive[index])
            continue;

        SpawnPointInternalData& internal_data = m_spawn_points_internal[index];
        if(!internal_data.active)
            continue;

        SpawnPointComponent& spawn_point = m_spawn_points[index];

        internal_data.counter += update_context.delta_ms;
        if(internal_data.counter < spawn_point.interval)
            continue;

        const float random_length = mono::Random(0.0f, spawn_point.radius);
        const math::Vector random_vector = math::VectorFromAngle(mono::Random(0.0f, math::PI() * 2.0f)) * random_length;

        math::Matrix world_transform = m_transform_system->GetWorld(index);
        math::Translate(world_transform, random_vector);

        SpawnEvent spawn_event;
        spawn_event.spawner_id = index;
        spawn_event.spawned_entity_id = 0;
        spawn_event.transform = world_transform;
        spawn_event.timestamp_to_spawn = update_context.timestamp + spawn_delay_time_ms;
        m_spawn_events.push_back(spawn_event);

        internal_data.counter = 0;
    }

    for(SpawnEvent& spawn_event : m_spawn_events)
    {
        // From spawn_score, lookup some entities to spawn. 
        //spawn_point.spawn_score;

        const bool time_to_spawn = (spawn_event.timestamp_to_spawn < update_context.timestamp);
        if(!time_to_spawn)
            continue;

        const SpawnDefinition&  spawn_definition = m_spawn_definitions.front();
        mono::Entity spawned_entity = m_entity_manager->CreateEntity(spawn_definition.entity_file.c_str());

        m_transform_system->SetTransform(spawned_entity.id, spawn_event.transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

        spawn_event.spawned_entity_id = spawned_entity.id;
    }
}

void SpawnSystem::Sync()
{
    const auto remove_if_spawned = [](const SpawnEvent& spawn_event) {
        return (spawn_event.spawned_entity_id != 0);
    };

    mono::remove_if(m_spawn_events, remove_if_spawned);
}
