
#pragma once

namespace game
{
    enum class WeaponType : int
    {
        STANDARD,
        FLAK_CANON,
        ROCKET_LAUNCHER,
        CACOPLASMA,
        GENERIC,
        LASER_BLASTER,

        TURRET,
        GRENADE,
    };

    constexpr int N_WEAPON_TYPES = static_cast<int>(WeaponType::LASER_BLASTER);

    inline bool IsBulletWeapon(WeaponType weapon_type)
    {
        return static_cast<int>(weapon_type) <= N_WEAPON_TYPES;
    }

    enum class WeaponFaction
    {
        PLAYER,
        ENEMY
    };
}
