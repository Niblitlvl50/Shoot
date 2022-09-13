
#pragma once

#include <cstdint>

namespace game
{
    enum class PackageAction
    {
        PICKED_UP,
        DROPPED,
        THROWN
    };

    struct PackagePickupEvent
    {
        constexpr PackagePickupEvent(uint32_t entity_id, uint32_t package_id, PackageAction package_action)
            : entity_id(entity_id)
            , package_id(package_id)
            , action(package_action)
        { }

        const uint32_t entity_id;
        const uint32_t package_id;
        const PackageAction action;
    };
}
