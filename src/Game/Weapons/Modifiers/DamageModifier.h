
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class DamageModifier : public IWeaponModifier
    {
    public:

        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;
    };
}
