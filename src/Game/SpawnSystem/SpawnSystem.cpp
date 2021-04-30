
#include "SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "TriggerSystem/TriggerSystem.h"

#include "Math/MathFunctions.h"
#include "Util/Hash.h"
#include "Util/Random.h"
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

SpawnSystem::SpawnPoint* SpawnSystem::AllocateSpawnPoint(uint32_t entity_id)
{
    assert(!m_alive[entity_id]);
    m_alive[entity_id] = true;

    SpawnPoint& spawn_point = m_spawn_points[entity_id];
    spawn_point.active = false;
    spawn_point.counter = 0;
    spawn_point.enable_callback_id = 0;
    spawn_point.disable_callback_id = 0;

    return &spawn_point;
}

void SpawnSystem::ReleaseSpawnPoint(uint32_t entity_id)
{
    m_alive[entity_id] = false;

    SpawnPoint& spawn_point = m_spawn_points[entity_id];
    if(spawn_point.enable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point.enable_trigger, spawn_point.enable_callback_id, entity_id);

    if(spawn_point.disable_callback_id != 0)
        m_trigger_system->RemoveTriggerCallback(spawn_point.disable_trigger, spawn_point.disable_callback_id, entity_id);
}

bool SpawnSystem::IsAllocated(uint32_t entity_id)
{
    return m_alive[entity_id];
}

void SpawnSystem::SetSpawnPointData(uint32_t entity_id, const SpawnSystem::SpawnPoint& component_data)
{
    assert(m_alive[sprite_id]);

    SpawnPoint& spawn_point = m_spawn_points[entity_id];
    spawn_point = component_data;

    if(spawn_point.enable_trigger != 0)
    {
        const game::TriggerCallback enable_callback = [&spawn_point](uint32_t entity_id) {
            spawn_point.active = true;
        };
        spawn_point.enable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point.enable_trigger, enable_callback, entity_id);
    }

    if(spawn_point.disable_trigger != 0)
    {
        const game::TriggerCallback disable_callback = [&spawn_point](uint32_t entity_id) {
            spawn_point.active = false;
        };
        spawn_point.disable_callback_id = m_trigger_system->RegisterTriggerCallback(spawn_point.disable_trigger, disable_callback, entity_id);
    }

    spawn_point.active = false;
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

        SpawnPoint& spawn_point = m_spawn_points[index];
        if(!spawn_point.active)
            continue;

        spawn_point.counter += update_context.delta_ms;
        if(spawn_point.counter < spawn_point.interval)
            continue;

        // From spawn_score, lookup some entities to spawn. 
        //spawn_point.spawn_score;

        const SpawnDefinition&  spawn_definition = m_spawn_definitions.front();
        mono::Entity spawned_entity = m_entity_manager->CreateEntity(spawn_definition.entity_file.c_str());

        const float random_length = mono::Random(0.0f, spawn_point.radius);
        const math::Vector random_vector = math::VectorFromAngle(mono::Random(0.0f, math::PI() * 2.0f)) * random_length;

        math::Matrix world_transform = m_transform_system->GetWorld(index);
        math::Translate(world_transform, random_vector);

        m_transform_system->SetTransform(spawned_entity.id, world_transform);
        m_transform_system->SetTransformState(spawned_entity.id, mono::TransformState::CLIENT);

        spawn_point.counter = 0;
    }
}
