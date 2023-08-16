
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Math/Vector.h"

#include <cstdint>

namespace game
{
    bool SeesPlayer(
        const mono::PhysicsSystem* physics_system, const math::Vector& position, const math::Vector& target_position);
}
