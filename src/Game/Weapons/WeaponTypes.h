
#pragma once

#include <cstdint>
#include <vector>

namespace game
{
    enum class WeaponFaction
    {
        PLAYER,
        ENEMY
    };

    struct WeaponSetup
    {
        WeaponSetup()
        { }

        WeaponSetup(uint32_t weapon_hash, uint32_t bullet_hash)
            : weapon_hash(weapon_hash)
            , bullet_hash(bullet_hash)
        { }

        uint32_t weapon_hash;
        uint32_t bullet_hash;
    };

    inline bool operator == (const WeaponSetup& left, const WeaponSetup& right)
    {
        return (left.weapon_hash == right.weapon_hash && left.bullet_hash == right.bullet_hash);
    }

    extern const WeaponSetup NO_WEAPON;
    extern const WeaponSetup GENERIC;
    extern const WeaponSetup PLASMA_GUN;
    extern const WeaponSetup FLAK_CANON;
    extern const WeaponSetup FLAK_CANON_PLASMA;
    extern const WeaponSetup CACO_PLASMA;
    extern const WeaponSetup CACO_PLASMA_HOMING;
    extern const WeaponSetup ROCKET_LAUNCHER;
    extern const WeaponSetup LASER_BLASTER;

    extern const WeaponSetup TURRET;

    std::vector<WeaponSetup> GetWeaponList();
    std::vector<WeaponSetup> GetSupportWeaponList();

    const char* GetWeaponSpriteFromHash(uint32_t weapon_hash);
}
