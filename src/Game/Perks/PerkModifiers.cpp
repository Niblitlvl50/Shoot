
#include "PerkModifiers.h"
#include "System/Hash.h"
#include "Weapons/WeaponConfiguration.h"

using namespace game;

WeaponFireRateModifier::WeaponFireRateModifier(const char* name_identifier, float multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_multiplier(multiplier)
{ }

uint32_t WeaponFireRateModifier::Id() const
{
    return m_id;
}

WeaponConfiguration WeaponFireRateModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration modified = weapon_config;
    modified.rounds_per_second *= m_multiplier;
    return modified;
}


BulletVelocityModifier::BulletVelocityModifier(const char* name_identifier, float multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_multiplier(multiplier)
{ }

uint32_t BulletVelocityModifier::Id() const
{
    return m_id;
}

WeaponConfiguration BulletVelocityModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration modified = weapon_config;
    modified.bullet_velocity *= m_multiplier;
    return modified;
}


BulletSpreadModifier::BulletSpreadModifier(const char* name_identifier, float multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_multiplier(multiplier)
{ }

uint32_t BulletSpreadModifier::Id() const
{
    return m_id;
}

WeaponConfiguration BulletSpreadModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration modified = weapon_config;
    modified.bullet_spread_degrees *= m_multiplier;
    return modified;
}


MagazineSizeModifier::MagazineSizeModifier(const char* name_identifier, int additional)
    : m_id(hash::Hash(name_identifier))
    , m_additional(additional)
{ }

uint32_t MagazineSizeModifier::Id() const
{
    return m_id;
}

WeaponConfiguration MagazineSizeModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration modified = weapon_config;
    modified.magazine_size += m_additional;
    return modified;
}


uint32_t InfiniteAmmoModifier::Id() const
{
    return hash::Hash("perk_infinite_ammo");
}

WeaponConfiguration InfiniteAmmoModifier::ModifyWeapon(const WeaponConfiguration& weapon_config)
{
    WeaponConfiguration modified = weapon_config;
    modified.infinite_ammo = true;
    return modified;
}


BulletRangeModifier::BulletRangeModifier(const char* name_identifier, float multiplier)
    : m_id(hash::Hash(name_identifier))
    , m_multiplier(multiplier)
{ }

uint32_t BulletRangeModifier::Id() const
{
    return m_id;
}

BulletConfiguration BulletRangeModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration modified = bullet_config;
    modified.life_span *= m_multiplier;
    return modified;
}


BulletMovementModifier::BulletMovementModifier(const char* name_identifier, uint32_t movement_flag)
    : m_id(hash::Hash(name_identifier))
    , m_movement_flag(movement_flag)
{ }

uint32_t BulletMovementModifier::Id() const
{
    return m_id;
}

BulletConfiguration BulletMovementModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration modified = bullet_config;
    modified.bullet_movement_behaviour |= m_movement_flag;
    return modified;
}
