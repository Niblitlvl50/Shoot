
#include "DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"

using namespace game;

WeaponConfiguration DamageModifier::ModifyWeapon(const WeaponConfiguration& WeaponConfig)
{
    return WeaponConfig;
}

BulletConfiguration DamageModifier::ModifyBullet(const BulletConfiguration& BulletConfig)
{
    return BulletConfig;
}
