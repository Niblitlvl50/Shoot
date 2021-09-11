
#pragma once

#include "WeaponTypes.h"
#include "WeaponConfiguration.h"
#include <memory>
#include <cstdint>

namespace game
{
    using IWeaponPtr = std::unique_ptr<class IWeapon>;

    class IWeaponFactory
    {
    public:
        virtual ~IWeaponFactory() = default;
        virtual IWeaponPtr CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id) = 0;
    };
}
