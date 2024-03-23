
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class DamageModifier : public IWeaponModifier
    {
    public:

        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& WeaponConfig) override;
        BulletConfiguration ModifyBullet(const BulletConfiguration& BulletConfig) override;
    };
}
