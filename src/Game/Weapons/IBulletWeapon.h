
#pragma once

#include "WeaponState.h"
#include "Math/MathFwd.h"
#include <cstdint>

namespace game
{
    class IBulletWeapon
    {
    public:

        virtual ~IBulletWeapon()
        { }

        virtual WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) = 0;
        virtual void Reload(uint32_t timestamp) = 0;
        virtual int AmmunitionLeft() const = 0;
        virtual int MagazineSize() const = 0;
        virtual WeaponState GetState() const = 0;
    };
}
