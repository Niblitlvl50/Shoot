
#pragma once

#include "Math/MathFwd.h"

namespace game
{
    enum class WeaponFireResult
    {
        NONE,
        FIRE,
        OUT_OF_AMMO
    };

    class IWeaponSystem
    {
    public:

        virtual ~IWeaponSystem()
        { }

        virtual WeaponFireResult Fire(const math::Vector& position, float direction) = 0;
        virtual int AmmunitionLeft() const = 0;        
    };
}
