
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"

namespace game
{
    struct PlayerInfo;

    bool SeesPlayer(
        const mono::PhysicsSystem* physics_system, const math::Vector& position, const game::PlayerInfo* player_info);
}
