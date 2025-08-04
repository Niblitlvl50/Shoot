
#include "DamageModifier.h"
#include "Weapons/WeaponConfiguration.h"
#include "System/Hash.h"

using namespace game;

DamageModifier::DamageModifier(const char* name_identifier, int add_damage_delta)
    : m_id(hash::Hash(name_identifier))
    , m_damage_delta(add_damage_delta)
    , m_damage_multiplier(1.0f)
{ }

DamageModifier::DamageModifier(const char* name_identifier, float damage_multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_damage_delta(0)
    , m_damage_multiplier(damage_multiplier)
{ }

uint32_t DamageModifier::Id() const
{
    return m_id;
}

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
    : m_id(hash::Hash("bullet_wall"))
{ }

uint32_t BulletWallModifier::Id() const
{
    return m_id;
}

WeaponConfiguration BulletWallModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration config = weapon_config;
    config.bullet_offset = 0.35f;

    return config;
}

CritChanceModifier::CritChanceModifier(const char* name_identifier, int percent_units)
    : m_id(hash::Hash(name_identifier))
    , m_percent_units(percent_units)
{ }

uint32_t CritChanceModifier::Id() const
{
    return m_id;
}

BulletConfiguration CritChanceModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration config = bullet_config;
    config.critical_hit_chance += m_percent_units;

    return config;
}
