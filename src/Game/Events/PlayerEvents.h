
#pragma once

#include "System/Network.h"

namespace game
{
    struct SpawnPlayerEvent
    {
        SpawnPlayerEvent(int player_index)
            : player_index(player_index)
        { }
        const int player_index;
    };

    struct DespawnPlayerEvent
    {
        DespawnPlayerEvent(int player_index)
            : player_index(player_index)
        { }

        const int player_index;
    };

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

    struct PlayerLevelUpEvent
    {
        PlayerLevelUpEvent(uint32_t entity_id)
            : entity_id(entity_id)
        { }
        const uint32_t entity_id;
    };
}
