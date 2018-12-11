
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <vector>

namespace game
{
    enum PickupTypes : int
    {
        AMMO,
        HEALTH,

        N_PICKUPS
    };

    struct Pickup
    {
        int value;
        int type;
        math::Vector position;
    };

    void CheckPlayerPickups(std::vector<Pickup>& pickups, mono::EventHandler& event_handler);
}
