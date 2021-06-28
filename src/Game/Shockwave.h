
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"

#include <cstdint>

namespace game
{
    class DamageSystem;

    void ShockwaveAt(mono::PhysicsSystem* physics_system, const math::Vector& world_position, float magnitude);
    void ShockwaveAndDamageAt(
        mono::PhysicsSystem* physics_system,
        game::DamageSystem* damage_system,
        const math::Vector& world_position,
        float magnitude,
        int damage,
        uint32_t who_did_damage);
}
