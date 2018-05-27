
#pragma once

#include "Math/MathFwd.h"

namespace game
{
    enum class WeaponFireResult
    {
        NONE,
        FIRE,
        RELOADING,
        OUT_OF_AMMO
    };

    class IWeaponSystem
    {
    public:

        virtual ~IWeaponSystem()
        { }

        virtual WeaponFireResult Fire(const math::Vector& position, float direction) = 0;
        virtual int AmmunitionLeft() const = 0;
        virtual int MagazineSize() const = 0;
        virtual void Reload() = 0;
    };
}
