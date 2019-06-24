
#pragma once

namespace game
{
    struct PlayerConnectedEvent
    {
        PlayerConnectedEvent(uint32_t id)
            : id(id)
        { }
        const uint32_t id;
    };

    struct PlayerDisconnectedEvent
    {
        PlayerDisconnectedEvent(uint32_t id)
            : id(id)
        { }
        const uint32_t id;
    };
}
