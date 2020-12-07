
#pragma once

#include "WeaponTypes.h"
#include <memory>
#include <cstdint>

namespace game
{
    class IBulletWeapon;
    class IThrowableWeapon;

    class IWeaponFactory
    {
    public:
        virtual ~IWeaponFactory() = default;
        virtual std::unique_ptr<IBulletWeapon> CreateWeapon(WeaponType weapon, WeaponFaction faction, uint32_t owner_id) = 0;
        virtual std::unique_ptr<IThrowableWeapon> CreateThrowable(ThrowableType weapon_type, WeaponFaction faction, uint32_t owner_id) = 0;
    };
}
