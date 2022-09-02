
#pragma once

#include "WeaponConfiguration.h"
#include "Util/StringFunctions.h"
#include "nlohmann/json.hpp"

namespace game
{
    inline void from_json(const nlohmann::json& json, BulletConfiguration& bullet_config)
    {
        bullet_config.name                  = json["name"].get<std::string>();
        bullet_config.damage                = json["damage"].get<int>();
        bullet_config.life_span             = json["life_span"].get<float>();
        bullet_config.fuzzy_life_span       = json["fuzzy_life_span"].get<float>();
        bullet_config.bullet_want_direction = json["bullet_want_direction"].get<bool>();
        bullet_config.entity_file           = json["entity_file"].get<std::string>();
        bullet_config.sound_file            = json["sound_file"].get<std::string>();
        bullet_config.bullet_behaviour      = 0;

        const std::string behaviour_string = json["behaviour"];
        const std::vector<std::string> parts = mono::SplitString(behaviour_string, '|');
        for(const std::string& flag_string : parts)
            bullet_config.bullet_behaviour |= StringToBulletCollisionFlag(flag_string.c_str());
    }

    inline void from_json(const nlohmann::json& json, WeaponConfiguration& weapon_config)
    {
        weapon_config.name                  = json["name"].get<std::string>();
        weapon_config.magazine_size         = json["magazine_size"].get<int>();
        weapon_config.projectiles_per_fire  = json["projectiles_per_fire"].get<int>();
        weapon_config.rounds_per_second     = json["rounds_per_second"].get<float>();
        weapon_config.fire_rate_multiplier  = json["fire_rate_multiplier"].get<float>();
        weapon_config.max_fire_rate         = json["max_fire_rate"].get<float>();
        weapon_config.bullet_force          = json["bullet_force"].get<float>();
        weapon_config.bullet_spread_degrees = json["bullet_spread_degrees"].get<float>();
        weapon_config.bullet_offset         = json["bullet_offset"].get<float>();
        weapon_config.bullet_force_random   = json["bullet_force_random"].get<bool>();
        weapon_config.reload_time           = json["reload_time"].get<float>();
        weapon_config.fire_sound            = json["fire_sound"].get<std::string>();
        weapon_config.out_of_ammo_sound     = json["out_of_ammo_sound"].get<std::string>();
        weapon_config.reload_sound          = json["reload_sound"].get<std::string>();
    }

    inline void from_json(const nlohmann::json& json, WeaponBulletCombination& weapon_setup)
    {
        weapon_setup.name           = json["name"].get<std::string>();
        weapon_setup.weapon         = json["weapon"].get<std::string>();
        weapon_setup.bullet         = json["bullet"].get<std::string>();
        weapon_setup.sprite_file    = json["sprite_file"].get<std::string>();
    }
}
