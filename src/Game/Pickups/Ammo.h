
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <vector>

namespace game
{
    enum class Pickups : int
    {
        AMMO,
        HEALTH
    };

    struct Ammo
    {
        int value;
        int type;
        math::Vector position;
    };

    void CheckPlayerPickups(std::vector<Ammo>& pickups, mono::EventHandler& event_handler);
}
