
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
        math::Vector position;
        float radius = 1.0f;
    };

    class Spawner
    {
    public:

        Spawner(const std::vector<SpawnPoint>& spawn_points, mono::EventHandler& eventHandler);

    private:

        const std::vector<SpawnPoint> m_spawn_points;
        std::unique_ptr<System::ITimer> m_timer;
    };
}
