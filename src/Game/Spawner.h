
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
        int time_stamp = 0;
        float radius = 1.0f;
        math::Vector position;
        std::vector<std::string> spawn_tags;
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

    private:

        void SpawnObject(const SpawnPoint& spawn_point);
        
        std::vector<SpawnPoint> m_spawn_points;
        std::vector<Wave> m_waves;
        mono::EventHandler& m_event_handler;

        int m_seconds_elapsed = 0;
        std::unique_ptr<System::ITimer> m_timer;
    };
}
