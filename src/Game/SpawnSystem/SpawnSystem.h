
#pragma once

#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "MonoFwd.h"

#include "Util/ActiveVector.h"
#include "System/Audio.h"

#include <vector>
#include <string>
#include <functional>

namespace game
{
    class SpawnSystem : public mono::IGameSystem
    {
    public:

        struct SpawnIdAndCallback
        {
            uint32_t spawned_entity_id;
            uint32_t callback_id;
        };

        struct SpawnPointComponent
        {
            int spawn_score;
            int interval_ms;

            // Zero means infinite
            int spawn_limit_concurrent;
            int spawn_limit_total;

            float radius;
            std::vector<math::Vector> points;

            uint32_t enable_trigger;
            uint32_t disable_trigger;

            // Internal data
            bool active;
            int counter_ms;
            int num_spawns;
            uint32_t enable_callback_id;
            uint32_t disable_callback_id;
            std::vector<SpawnIdAndCallback> active_spawns;
        };

        struct EntitySpawnPointComponent
        {
            std::string entity_file;
            float radius;
            uint32_t spawn_trigger;

            // Internal data
            uint32_t entity_id;
            uint32_t callback_id;
        };

        struct DespawnEntityComponent
        {
            uint32_t despawn_trigger_hash;

            // Internal data
            uint32_t entity_id;
            uint32_t callback_id;
        };

        struct SpawnDefinition
        {
            int value;
            std::string entity_file;
        };

        struct SpawnEvent
        {
            uint32_t spawner_id;
            uint32_t spawned_entity_id;
            math::Matrix transform;
            uint32_t timestamp_to_spawn;

            int spawn_score;
            std::string entity_file;
        };

        using SpawnCallback = std::function<void (uint32_t entity_id, int spawn_score)>;

        static const uint32_t spawn_delay_time_ms = 500;

        SpawnSystem(uint32_t n, mono::TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system);

        SpawnPointComponent* AllocateSpawnPoint(uint32_t entity_id);
        void ReleaseSpawnPoint(uint32_t entity_id);
        bool IsAllocated(uint32_t entity_id);
        void SetSpawnPointData(uint32_t entity_id, const SpawnPointComponent& component_data);

        EntitySpawnPointComponent* AllocateEntitySpawnPoint(uint32_t entity_id);
        void ReleaseEntitySpawnPoint(uint32_t entity_id);
        void SetEntitySpawnPointData(uint32_t entity_id, const std::string& entity_file, float spawn_radius, uint32_t spawn_trigger);

        void AllocateDespawnTrigger(uint32_t entity_id);
        void ReleaseDespawnTrigger(uint32_t entity_id);
        void SetDespawnTriggerData(uint32_t entity_id, uint32_t despawn_trigger_hash);

        int GetActiveSpawns(const SpawnPointComponent* spawn_point);
        const std::vector<SpawnEvent>& GetSpawnEvents() const;

        uint32_t AddGlobalSpawnCallback(const SpawnCallback& callback);
        void RemoveGlobalSpawnCallback(uint32_t callback_id);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Sync() override;
        void Reset() override;

        template <typename T>
        inline void ForEachSpawnPoint(T&& func)
        {
            m_spawn_points.ForEach(func);
        }

        template <typename T>
        inline void ForEachEntitySpawnPoint(T&& func)
        {
            m_entity_spawn_points.ForEach(func);
        }

    private:

        mono::TriggerSystem* m_trigger_system;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;

        std::vector<audio::ISoundPtr> m_spawn_sounds;
        std::vector<SpawnDefinition> m_spawn_definitions;

        mono::ActiveVector<SpawnPointComponent> m_spawn_points;
        mono::ActiveVector<EntitySpawnPointComponent> m_entity_spawn_points;
        mono::ActiveVector<DespawnEntityComponent> m_despawn_entity_triggers;
        std::vector<EntitySpawnPointComponent*> m_active_entity_spawn_points;

        std::vector<SpawnEvent> m_spawn_events;

        struct SpawnCallbackData
        {
            SpawnCallback callback;
        };
        SpawnCallbackData m_spawn_callbacks[8];
    };
}
