
#include "DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"

using namespace game;

DamageModifier::DamageModifier(int add_damage_delta)
    : m_damage_delta(add_damage_delta)
    , m_damage_multiplier(1.0f)
{ }

DamageModifier::DamageModifier(float damage_multiplier)
    : m_damage_delta(0)
    , m_damage_multiplier(damage_multiplier)
{ }

BulletConfiguration DamageModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration config = bullet_config;

    config.min_damage += m_damage_delta;
    config.max_damage += m_damage_delta;

    config.min_damage *= m_damage_multiplier;
    config.max_damage *= m_damage_multiplier;

    return config;
}



BulletWallModifier::BulletWallModifier()
{ }

WeaponConfiguration BulletWallModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration config = weapon_config;

    config.rounds_per_second *= 2.0f;
    config.bullet_offset = 0.35f;
    config.infinite_ammo = true;

    return config;
}

