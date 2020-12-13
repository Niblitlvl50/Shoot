
#pragma once

#include "System/Network.h"

namespace game
{
    struct SpawnPlayerEvent
    {};

    struct PlayerConnectedEvent
    {
        PlayerConnectedEvent(const network::Address& address)
            : address(address)
        { }
        const network::Address address;
    };

    struct PlayerDisconnectedEvent
    {
        PlayerDisconnectedEvent(const network::Address& address)
            : address(address)
        { }
        const network::Address address;
    };
}
