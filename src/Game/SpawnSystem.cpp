
#include "SpawnSystem.h"
#include "Util/Hash.h"

#include <cassert>

using namespace game;

SpawnSystem::SpawnSystem(size_t n, mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
{
    m_spawn_points.resize(n);
    m_alive.resize(n, false);
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

}
