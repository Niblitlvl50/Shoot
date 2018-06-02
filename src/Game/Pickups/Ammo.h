
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <vector>

namespace game
{
    struct Ammo
    {
        int value;
        math::Vector position;
    };

    void CheckPlayerPickups(std::vector<Ammo>& pickups, mono::EventHandler& event_handler);
}
