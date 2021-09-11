
#pragma once

#include <cstdint>

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
        {}

        WeaponSetup(uint32_t weapon_hash, uint32_t bullet_hash)
            : weapon_hash(weapon_hash)
            , bullet_hash(bullet_hash)
        {}

        uint32_t weapon_hash;
        uint32_t bullet_hash;
    };

    extern const WeaponSetup GENERIC;
    extern const WeaponSetup PLASMA_GUN;
    extern const WeaponSetup FLAK_CANON;
    extern const WeaponSetup CACO_PLASMA;
    extern const WeaponSetup ROCKET_LAUNCHER;
    extern const WeaponSetup LASER_BLASTER;

    extern const WeaponSetup TURRET;


    static const WeaponSetup g_weapon_list[] = {
        GENERIC,
        PLASMA_GUN,
        FLAK_CANON,
        CACO_PLASMA,
        ROCKET_LAUNCHER,
        LASER_BLASTER
    };
}
