
#pragma once

#include <cstdint>

namespace game
{
    enum class PickupType : int
    {
        AMMO,
        HEALTH,
        SECOND_WIND,
        WEAPON_PISTOL,
        WEAPON_PLASMA,
        WEAPON_SHOTGUN
    };

    constexpr const char* pickup_items[] = {
        "Ammo",
        "Health",
        "Second Wind",
        "Pistol",
        "Plasma Gun",
        "Shotgun",
    };

    inline const char* PickupTypeToString(PickupType pickup_type)
    {
        return pickup_items[static_cast<int>(pickup_type)];
    }
}
