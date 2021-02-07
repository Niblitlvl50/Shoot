
#pragma once

#include "System/Network.h"

namespace game
{
    struct SpawnPlayerEvent
    {};

    struct RespawnPlayerEvent
    {
        RespawnPlayerEvent(uint32_t entity_id)
            : entity_id(entity_id)
        { }
        const uint32_t entity_id;
    };

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
