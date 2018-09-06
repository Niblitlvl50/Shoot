
#pragma once

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
}
