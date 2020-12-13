
#pragma once

#include "EventHandler/EventToken.h"
#include <functional>

namespace game
{
    struct ShockwaveEvent;
    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;
    struct SpawnPlayerEvent;
    struct ScoreEvent;

    using ShockwaveFunc = std::function<mono::EventResult (const game::ShockwaveEvent&)>;
    using PlayerConnectedFunc = std::function<mono::EventResult (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<mono::EventResult (const game::PlayerDisconnectedEvent&)>;
    using SpawnPlayerFunc = std::function<mono::EventResult (const game::SpawnPlayerEvent&)>;
    using ScoreFunc = std::function<mono::EventResult (const game::ScoreEvent&)>;
}
