
#include "PrefabSystem.h"

#include "EntitySystem/IEntityManager.h"
#include "EntitySystem/Entity.h"
#include "TransformSystem/TransformSystem.h"
#include "Math/Quad.h"
#include "System/System.h"

#include <algorithm>
#include <iterator>

namespace
{
    constexpr int max_prefab_depth = 8;

    bool SameTransform(const math::Matrix& first, const math::Matrix& second)
    {
        return std::equal(std::begin(first.data), std::end(first.data), std::begin(second.data));
    }
}

using namespace game;

PrefabSystem::PrefabSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system)
    : m_entity_manager(entity_manager)
    , m_transform_system(transform_system)
{ }

void PrefabSystem::AllocatePrefab(uint32_t entity_id)
{
    m_prefabs[entity_id] = PrefabComponent();
}

void PrefabSystem::ReleasePrefab(uint32_t entity_id)
{
    const auto it = m_prefabs.find(entity_id);
    if(it == m_prefabs.end())
        return;

    ReleaseContents(it->second);
    m_prefabs.erase(it);

    // Back to the default size of an entity, the contents it covered are gone.
    m_transform_system->SetBoundingBox(entity_id, math::Quad(-0.5f, -0.5f, 0.5f, 0.5f));
}

void PrefabSystem::SetPrefabData(uint32_t entity_id, const std::string& entity_file)
{
    const auto it = m_prefabs.find(entity_id);
    if(it == m_prefabs.end())
        return;

    PrefabComponent& prefab = it->second;
    if(prefab.entity_file == entity_file && (prefab.needs_spawn || !prefab.spawned.empty()))
        return;

    // Spawned in the next update, when the prefab's transform is set too.
    ReleaseContents(prefab);
    prefab.entity_file = entity_file;
    prefab.needs_spawn = !entity_file.empty();
}

const char* PrefabSystem::Name() const
{
    return "prefabsystem";
}

void PrefabSystem::Update(const mono::UpdateContext& update_context)
{
    for(auto& [prefab_entity_id, prefab] : m_prefabs)
    {
        if(prefab.needs_spawn)
        {
            prefab.needs_spawn = false;
            SpawnContents(prefab_entity_id, prefab);
        }

        // Only moved again when the prefab moves, otherwise the contents are left alone.
        if(!prefab.spawned.empty())
        {
            const math::Matrix& prefab_transform = m_transform_system->GetWorld(prefab_entity_id);
            if(!SameTransform(prefab_transform, prefab.applied_transform))
                ApplyTransform(prefab_entity_id, prefab);

            UpdateBoundingBox(prefab_entity_id, prefab);
        }
    }
}

void PrefabSystem::SpawnContents(uint32_t prefab_entity_id, PrefabComponent& prefab)
{
    if(IsRecursive(prefab_entity_id, prefab.entity_file))
    {
        System::Log("PrefabSystem|'%s' spawns itself through its prefabs, it's not spawned again.", prefab.entity_file.c_str());
        return;
    }

    const std::vector<mono::Entity> spawned_entities = m_entity_manager->SpawnEntityCollection(prefab.entity_file.c_str());

    for(const mono::Entity& spawned_entity : spawned_entities)
    {
        // Something spawned by the prefab that gets released on its own, a killed enemy for example,
        // is dropped from the prefab.
        const mono::ReleaseCallback on_released = [this, prefab_entity_id](uint32_t released_entity_id, mono::ReleasePhase phase) {
            m_spawned_by.erase(released_entity_id);

            const auto it = m_prefabs.find(prefab_entity_id);
            if(it == m_prefabs.end())
                return;

            std::vector<SpawnedEntity>& spawned = it->second.spawned;
            for(auto spawned_it = spawned.begin(); spawned_it != spawned.end(); ++spawned_it)
            {
                if(spawned_it->entity_id == released_entity_id)
                {
                    spawned.erase(spawned_it);
                    break;
                }
            }
        };

        SpawnedEntity spawned;
        spawned.entity_id = spawned_entity.id;
        spawned.local_transform = m_transform_system->GetTransform(spawned_entity.id);
        spawned.release_callback_id = m_entity_manager->AddReleaseCallback(spawned_entity.id, mono::ReleasePhase::PRE_RELEASE, on_released);

        prefab.spawned.push_back(spawned);
        m_spawned_by[spawned_entity.id] = prefab_entity_id;
    }

    ApplyTransform(prefab_entity_id, prefab);
    UpdateBoundingBox(prefab_entity_id, prefab);
}

void PrefabSystem::ReleaseContents(PrefabComponent& prefab)
{
    for(const SpawnedEntity& spawned : prefab.spawned)
    {
        m_entity_manager->RemoveReleaseCallback(spawned.entity_id, spawned.release_callback_id);
        m_entity_manager->ReleaseEntity(spawned.entity_id);
        m_spawned_by.erase(spawned.entity_id);
    }

    prefab.spawned.clear();
}

void PrefabSystem::ApplyTransform(uint32_t prefab_entity_id, PrefabComponent& prefab)
{
    const math::Matrix& prefab_transform = m_transform_system->GetWorld(prefab_entity_id);

    for(const SpawnedEntity& spawned : prefab.spawned)
    {
        const math::Matrix world_transform = prefab_transform * spawned.local_transform;
        m_transform_system->SetTransform(spawned.entity_id, world_transform, mono::TransformState::CLIENT);
    }

    prefab.applied_transform = prefab_transform;
}

void PrefabSystem::UpdateBoundingBox(uint32_t prefab_entity_id, const PrefabComponent& prefab)
{
    // The contents' bounds in the prefab's own space, as the editor picks and outlines with them.
    const math::Matrix to_prefab_space = math::Inverse(m_transform_system->GetWorld(prefab_entity_id));

    math::Quad bounds = math::InverseInfQuad;
    for(const SpawnedEntity& spawned : prefab.spawned)
    {
        const math::Quad world_bounds = m_transform_system->GetWorldBoundingBox(spawned.entity_id);
        math::ExpandBy(bounds, math::Transformed(to_prefab_space, world_bounds));
    }

    if(!(bounds == math::InverseInfQuad))
        m_transform_system->SetBoundingBox(prefab_entity_id, bounds);
}

bool PrefabSystem::IsRecursive(uint32_t prefab_entity_id, const std::string& entity_file) const
{
    uint32_t current = prefab_entity_id;

    for(int depth = 0; depth < max_prefab_depth; ++depth)
    {
        const auto spawned_by_it = m_spawned_by.find(current);
        if(spawned_by_it == m_spawned_by.end())
            return false;

        const uint32_t parent_prefab_id = spawned_by_it->second;
        const auto prefab_it = m_prefabs.find(parent_prefab_id);
        if(prefab_it != m_prefabs.end() && prefab_it->second.entity_file == entity_file)
            return true;

        current = parent_prefab_id;
    }

    // Deeper than any sensible nesting, treat it as recursion.
    return true;
}
