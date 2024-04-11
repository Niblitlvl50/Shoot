
#pragma once

#include "WeaponConfiguration.h"

#include <cstdint>
#include <string>
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
            : weapon_identifier_hash(0)
            , weapon_hash(0)
            , bullet_hash(0)
        { }

        WeaponSetup(uint32_t weapon_identifier_hash, uint32_t weapon_hash, uint32_t bullet_hash)
            : weapon_identifier_hash(weapon_identifier_hash)
            , weapon_hash(weapon_hash)
            , bullet_hash(bullet_hash)
        { }

        uint32_t weapon_identifier_hash;
        uint32_t weapon_hash;
        uint32_t bullet_hash;
    };

    inline bool operator == (const WeaponSetup& left, const WeaponSetup& right)
    {
        return (left.weapon_identifier_hash == right.weapon_identifier_hash && left.weapon_hash == right.weapon_hash && left.bullet_hash == right.bullet_hash);
    }

    struct WeaponConfig
    {
        std::string weapon_pickup_entity;
        std::unordered_map<uint32_t, struct BulletConfiguration> bullet_configs;
        std::unordered_map<uint32_t, struct WeaponConfiguration> weapon_configs;
        std::unordered_map<uint32_t, struct WeaponBulletCombination> weapon_combinations;
        std::vector<std::string> weapon_names;
    };

    WeaponConfig LoadWeaponConfig(const char* weapon_config);
    WeaponSetup FindWeaponSetupFromString(const WeaponConfig& weapon_config, const char* weapon_combination_name);
}
