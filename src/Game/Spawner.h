
#pragma once

#include "IGameSystem.h"
#include "System/System.h"
#include "Math/Vector.h"
#include "MonoFwd.h"

#include <memory>
#include <vector>
#include <string>

namespace game
{
    struct SpawnPoint
    {
        float radius = 1.0f;
        math::Vector position;
    };

    struct Wave
    {
        std::string name;
        std::vector<std::string> tags;
    };

    std::vector<Wave> LoadWaveFile(const char* wave_file);

    class Spawner
    {
    public:

        Spawner(
            const std::vector<SpawnPoint>& spawn_points, const std::vector<Wave>& waves, mono::EventHandler& event_handler);
        void CheckForSpawn();
        void EntityDestroyed(unsigned int spawn_id);

    private:

        void SpawnNextWave();

        std::vector<SpawnPoint> m_spawn_points;
        std::vector<Wave> m_waves;
        mono::EventHandler& m_event_handler;
        std::unique_ptr<System::ITimer> m_timer;

        int m_wave_index;
        std::vector<unsigned int> m_current_spawned_ids;
    };

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
