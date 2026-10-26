
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Math/Matrix.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace game
{
    // A prefab is an entity in a level that refers to an entity file instead of holding the data
    // itself. Its contents are spawned from the file when the level loads, so changing the file
    // changes every level that places it. The level only stores the prefab entity.
    //
    // The spawned entities are placed at the prefab's transform but not parented to it, so physics in
    // them keeps working, they're only moved again if the prefab itself moves (in the editor). They are
    // released with the prefab, and the prefab's bounding box covers them so it can be picked in the
    // editor.
    class PrefabSystem : public mono::IGameSystem
    {
    public:

        PrefabSystem(mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system);

        void AllocatePrefab(uint32_t entity_id);
        void ReleasePrefab(uint32_t entity_id);
        void SetPrefabData(uint32_t entity_id, const std::string& entity_file);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        struct SpawnedEntity
        {
            uint32_t entity_id;
            uint32_t release_callback_id;
            math::Matrix local_transform;
        };

        struct PrefabComponent
        {
            std::string entity_file;
            bool needs_spawn = false;
            std::vector<SpawnedEntity> spawned;
            math::Matrix applied_transform;
        };

        void SpawnContents(uint32_t prefab_entity_id, PrefabComponent& prefab);
        void ReleaseContents(PrefabComponent& prefab);
        void ApplyTransform(uint32_t prefab_entity_id, PrefabComponent& prefab);
        void UpdateBoundingBox(uint32_t prefab_entity_id, const PrefabComponent& prefab);

        // True if spawning entity_file for this prefab would spawn a file that's already further up
        // the chain of prefabs that spawned it.
        bool IsRecursive(uint32_t prefab_entity_id, const std::string& entity_file) const;

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;

        std::unordered_map<uint32_t, PrefabComponent> m_prefabs;

        // Spawned entity -> the prefab that spawned it, for finding self referencing files.
        std::unordered_map<uint32_t, uint32_t> m_spawned_by;
    };
}
