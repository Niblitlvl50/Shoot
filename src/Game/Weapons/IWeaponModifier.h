
#pragma once

#include "Weapons/WeaponConfiguration.h"
#include <cstdint>

namespace game
{
    class IWeaponModifier
    {
    public:

        virtual ~IWeaponModifier() = default;

        uint32_t Id() const;
        virtual WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) { return weapon_config; }
        virtual BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) { return bullet_config; }
    };
}
