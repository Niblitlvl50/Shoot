
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class DamageModifier : public IWeaponModifier
    {
    public:

        DamageModifier(int add_damage_delta);
        DamageModifier(float damage_multiplier);

        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;

        int m_damage_delta;
        float m_damage_multiplier;
    };

    class BulletWallModifier : public IWeaponModifier
    {
    public:

        BulletWallModifier();
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    };
}
