
#pragma once

#include "Math/MathFwd.h"

namespace game
{
    class IParticleEffect
    {
    public:

        virtual ~IParticleEffect() = default;

        virtual void EmitAt(const math::Vector& world_position) = 0;
        virtual void EmitAtWithDirection(const math::Vector& world_position, float direction) = 0;
    };
}
