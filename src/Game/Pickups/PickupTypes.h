
#pragma once

#include <cstdint>

namespace game
{
    enum class PickupType : int
    {
        AMMO,
        HEALTH,
        SECOND_WIND,
        COINS,
    };

    constexpr const char* g_pickup_items[] = {
        "Ammo",
        "Health",
        "Second Wind",
        "Coins",
    };

    inline const char* PickupTypeToString(PickupType pickup_type)
    {
        return g_pickup_items[static_cast<int>(pickup_type)];
    }
}
