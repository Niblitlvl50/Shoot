
#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>

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

    struct WeaponConfig
    {
        std::unordered_map<uint32_t, struct BulletConfiguration> bullet_configs;
        std::unordered_map<uint32_t, struct WeaponConfiguration> weapon_configs;
        std::unordered_map<uint32_t, struct WeaponBulletCombination> weapon_combinations;
        std::vector<std::string> weapon_names;
    };

    WeaponConfig LoadWeaponConfig(const char* weapon_config);
}
