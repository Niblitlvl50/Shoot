
#pragma once

#include "IGameSystem.h"
#include "Math/Vector.h"
#include "MonoFwd.h"

#include <vector>

namespace game
{
    class SpawnSystem : public mono::IGameSystem
    {
    public:

        struct SpawnPoint
        {
            int spawn_score;
        };

        SpawnSystem(size_t n, mono::TransformSystem* transform_system);

        SpawnPoint* AllocateSpawnPoint(uint32_t entity_id);
        bool IsAllocated(uint32_t entity_id);
        void SetSpawnPointData(uint32_t entity_id, const SpawnPoint& component_data);
        void ReleaseSpawnPoint(uint32_t entity_id);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::TransformSystem* m_transform_system;

        std::vector<SpawnPoint> m_spawn_points;
        std::vector<bool> m_alive;
    };
}
