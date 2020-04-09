
#pragma once

#include "EventHandler/EventToken.h"
#include <functional>

namespace game
{
    struct ShockwaveEvent;
    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;

    using ShockwaveFunc = std::function<mono::EventResult (const game::ShockwaveEvent&)>;
    using PlayerConnectedFunc = std::function<mono::EventResult (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<mono::EventResult (const game::PlayerDisconnectedEvent&)>;
}
