
#include "SpawnSystem.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Hash.h"
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
    return &m_spawn_points[entity_id];
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
}

void SpawnSystem::ReleaseSpawnPoint(uint32_t entity_id)
{
    m_alive[entity_id] = false;
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

        continue;

        // Check for spawn.
        const SpawnPoint& spawn_point = m_spawn_points[index];

        // Time to spawn?
        // From spawn_score, lookup some entities to spawn. 
        spawn_point.spawn_score;

        const math::Matrix& world_transform = m_transform_system->GetWorld(index);

        mono::Entity spawned_entity = m_entity_manager->CreateEntity("hello");
        m_transform_system->SetTransform(spawned_entity.id, world_transform);

    }
}
