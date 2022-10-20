
#pragma once

#include "WeaponState.h"
#include "WeaponTypes.h"
#include "Math/MathFwd.h"
#include <cstdint>

namespace game
{
    class IWeapon
    {
    public:

        virtual ~IWeapon()
        { }

        virtual WeaponState Fire(const math::Vector& position, float direction, uint32_t timestamp) = 0;
        virtual WeaponState Fire(const math::Vector& position, const math::Vector& target, uint32_t timestamp) = 0;

        virtual void StopFire(uint32_t timestamp) = 0;
        virtual void Reload(uint32_t timestamp) = 0;
        virtual WeaponState UpdateWeaponState(uint32_t timestamp) = 0;
        virtual WeaponState GetWeaponState() const = 0;
        virtual void AddAmmunition(int amount) = 0;

        virtual int AmmunitionLeft() const = 0;
        virtual int MagazineSize() const = 0;
        virtual int ReloadPercentage() const = 0;

        virtual WeaponSetup GetWeaponSetup() const = 0;
    };
}
