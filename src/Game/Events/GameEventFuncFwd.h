
#pragma once

#include "EventHandler/EventToken.h"
#include <functional>

namespace game
{
    struct ShockwaveEvent;
    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;
    struct SpawnPlayerEvent;
    struct DespawnPlayerEvent;
    struct RespawnPlayerEvent;
    struct AddDrawableEvent;
    struct RemoveDrawableEvent;
    struct GameOverEvent;

    using ShockwaveFunc = std::function<mono::EventResult (const game::ShockwaveEvent&)>;
    using PlayerConnectedFunc = std::function<mono::EventResult (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<mono::EventResult (const game::PlayerDisconnectedEvent&)>;
    using SpawnPlayerFunc = std::function<mono::EventResult (const game::SpawnPlayerEvent&)>;
    using DespawnPlayerFunc = std::function<mono::EventResult (const game::DespawnPlayerEvent&)>;
    using RespawnPlayerFunc = std::function<mono::EventResult (const game::RespawnPlayerEvent&)>;

    using AddDrawableFunc = std::function<mono::EventResult (const game::AddDrawableEvent&)>;
    using RemoveDrawableFunc = std::function<mono::EventResult (const game::RemoveDrawableEvent&)>;

    using GameOverFunc = std::function<mono::EventResult (const game::GameOverEvent&)>;
}
