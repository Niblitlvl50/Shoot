
#pragma once

#include "System/System.h"
#include "Math/Vector.h"
#include "MonoFwd.h"
#include <memory>
#include <vector>

namespace game
{
    struct SpawnPoint
    {
        int intervall;
        float radius = 1.0f;
        math::Vector position;
        std::vector<std::string> spawn_names;
    };

    class Spawner
    {
    public:

        Spawner(const std::vector<SpawnPoint>& spawn_points, mono::EventHandler& event_handler);
        void SpawnObject();

    private:
        
        const std::vector<SpawnPoint> m_spawn_points;
        mono::EventHandler& m_event_handler;
        std::unique_ptr<System::ITimer> m_timer;
    };
}
