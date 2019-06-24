
#pragma once

#include <functional>

namespace game
{
    struct DamageEvent;
    struct RemoveEntityEvent;
    struct ShockwaveEvent;
    struct SpawnConstraintEvent;
    struct DespawnConstraintEvent;
    struct SpawnEntityEvent;
    struct SpawnPhysicsEntityEvent;

    struct PlayerConnectedEvent;
    struct PlayerDisconnectedEvent;


    using DamageFunc = std::function<bool (const game::DamageEvent&)>; 
    using RemoveEntityFunc = std::function<bool (const game::RemoveEntityEvent&)>;
    using ShockwaveFunc = std::function<bool (const game::ShockwaveEvent&)>;
    using SpawnConstraintFunc = std::function<bool (const game::SpawnConstraintEvent&)>;
    using DespawnConstraintFunc = std::function<bool (const game::DespawnConstraintEvent&)>;
    using SpawnEntityFunc = std::function<bool (const game::SpawnEntityEvent&)>;
    using SpawnPhysicsEntityFunc = std::function<bool (const game::SpawnPhysicsEntityEvent&)>;

    using PlayerConnectedFunc = std::function<bool (const game::PlayerConnectedEvent&)>; 
    using PlayerDisconnectedFunc = std::function<bool (const game::PlayerDisconnectedEvent&)>;
}
