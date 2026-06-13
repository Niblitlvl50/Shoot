
#pragma once

#include "Weapons/IWeaponModifier.h"
#include <cstdint>

namespace game
{
    class WeaponFireRateModifier : public IWeaponModifier
    {
    public:
        WeaponFireRateModifier(const char* name_identifier, float multiplier);
        uint32_t Id() const override;
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    private:
        const uint32_t m_id;
        const float m_multiplier;
    };

    class BulletVelocityModifier : public IWeaponModifier
    {
    public:
        BulletVelocityModifier(const char* name_identifier, float multiplier);
        uint32_t Id() const override;
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    private:
        const uint32_t m_id;
        const float m_multiplier;
    };

    class BulletSpreadModifier : public IWeaponModifier
    {
    public:
        BulletSpreadModifier(const char* name_identifier, float multiplier);
        uint32_t Id() const override;
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    private:
        const uint32_t m_id;
        const float m_multiplier;
    };

    class MagazineSizeModifier : public IWeaponModifier
    {
    public:
        MagazineSizeModifier(const char* name_identifier, int additional);
        uint32_t Id() const override;
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    private:
        const uint32_t m_id;
        const int m_additional;
    };

    class InfiniteAmmoModifier : public IWeaponModifier
    {
    public:
        uint32_t Id() const override;
        WeaponConfiguration ModifyWeapon(const WeaponConfiguration& weapon_config) override;
    };

    class BulletRangeModifier : public IWeaponModifier
    {
    public:
        BulletRangeModifier(const char* name_identifier, float multiplier);
        uint32_t Id() const override;
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;
    private:
        const uint32_t m_id;
        const float m_multiplier;
    };

    class BulletMovementModifier : public IWeaponModifier
    {
    public:
        BulletMovementModifier(const char* name_identifier, uint32_t movement_flag);
        uint32_t Id() const override;
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;
    private:
        const uint32_t m_id;
        const uint32_t m_movement_flag;
    };
}
