
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <vector>

namespace game
{
    enum Pickups : int
    {
        AMMO,
        HEALTH,

        N_AMMOS
    };

    struct Ammo
    {
        int value;
        int type;
        math::Vector position;
    };

    void CheckPlayerPickups(std::vector<Ammo>& pickups, mono::EventHandler& event_handler);
}
