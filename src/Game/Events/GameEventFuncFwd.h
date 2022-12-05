
#pragma once

#include "EventHandler/EventToken.h"
#include <functional>

namespace game
{
    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;
    struct SpawnPlayerEvent;
    struct DespawnPlayerEvent;
    struct RespawnPlayerEvent;
    struct GameOverEvent;
    struct PackagePickupEvent;

    using PlayerConnectedFunc = std::function<mono::EventResult (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<mono::EventResult (const game::PlayerDisconnectedEvent&)>;
    using SpawnPlayerFunc = std::function<mono::EventResult (const game::SpawnPlayerEvent&)>;
    using DespawnPlayerFunc = std::function<mono::EventResult (const game::DespawnPlayerEvent&)>;
    using RespawnPlayerFunc = std::function<mono::EventResult (const game::RespawnPlayerEvent&)>;
    using PackagePickupFunc = std::function<mono::EventResult (const game::PackagePickupEvent&)>;

    using GameOverFunc = std::function<mono::EventResult (const game::GameOverEvent&)>;
}
