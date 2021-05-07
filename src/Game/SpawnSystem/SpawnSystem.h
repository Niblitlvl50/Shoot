
#pragma once

#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Math/Matrix.h"
#include "MonoFwd.h"

#include <vector>
#include <string>

namespace game
{
    class SpawnSystem : public mono::IGameSystem
    {
    public:

        struct SpawnPoint
        {
            int spawn_score;
            float radius;
            int interval;
            uint32_t properties;

            uint32_t enable_trigger;
            uint32_t disable_trigger;

            // internal data
            bool active;
            int counter;
            uint32_t enable_callback_id;
            uint32_t disable_callback_id;
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
        };

        static const uint32_t spawn_delay_time_ms = 500;

        SpawnSystem(uint32_t n, class TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system);

        SpawnPoint* AllocateSpawnPoint(uint32_t entity_id);
        void ReleaseSpawnPoint(uint32_t entity_id);
        bool IsAllocated(uint32_t entity_id);
        void SetSpawnPointData(uint32_t entity_id, const SpawnPoint& component_data);

        const std::vector<SpawnEvent>& GetSpawnEvents() const;


        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Sync() override;

        template <typename T>
        inline void ForEeach(T&& func)
        {
            for(uint32_t entity_id = 0; entity_id < m_spawn_points.size(); ++entity_id)
            {
                if(m_alive[entity_id])
                    func(entity_id, m_spawn_points[entity_id]);
            }
        }

        game::TriggerSystem* m_trigger_system;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        std::vector<SpawnDefinition> m_spawn_definitions;
        std::vector<SpawnPoint> m_spawn_points;
        std::vector<bool> m_alive;

        std::vector<SpawnEvent> m_spawn_events;
    };
}
