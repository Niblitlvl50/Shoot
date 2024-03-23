
#pragma once

#include "Weapons/WeaponConfiguration.h"

namespace game
{
    class IWeaponModifier
    {
    public:

        virtual ~IWeaponModifier() = default;

        virtual WeaponConfiguration ModifyWeapon(const WeaponConfiguration& WeaponConfig) { return WeaponConfig; }
        virtual BulletConfiguration ModifyBullet(const BulletConfiguration& BulletConfig) { return BulletConfig; }
    };
}
