
#include "DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"

using namespace game;

WeaponConfiguration DamageModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    return weapon_config;
}

BulletConfiguration DamageModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration config = bullet_config;
    config.min_damage += 25;
    config.max_damage += 50;
    return config;
}
