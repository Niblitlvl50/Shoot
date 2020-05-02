
#pragma once

#include "WeaponTypes.h"
#include <memory>
#include <cstdint>

namespace game
{
    class IWeaponSystem;

    class IWeaponFactory
    {
    public:
        virtual ~IWeaponFactory()
        { }

        virtual std::unique_ptr<IWeaponSystem> CreateWeapon(WeaponType weapon, WeaponFaction faction, uint32_t owner_id) = 0;
    };
}
