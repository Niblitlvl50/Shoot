
#pragma once

#include <functional>

namespace game
{
    struct ShockwaveEvent;
    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;

    using ShockwaveFunc = std::function<bool (const game::ShockwaveEvent&)>;
    using PlayerConnectedFunc = std::function<bool (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<bool (const game::PlayerDisconnectedEvent&)>;
}
