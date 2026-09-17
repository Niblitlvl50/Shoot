
#pragma once

#include "PlayerConfig.h"
#include "nlohmann/json.hpp"

namespace game
{
    inline void from_json(const nlohmann::json& json, PlayerConfig& config)
    {
        if(json.contains("player_entities"))
            config.player_entities = json["player_entities"].get<std::vector<std::string>>();

        if(json.contains("train_entities"))
            config.train_entities = json["train_entities"].get<std::vector<std::string>>();

            if(json.contains("familiar_entities"))
            config.familiar_entities = json["familiar_entities"].get<std::vector<std::string>>();

        if(json.contains("package_entities"))
            config.package_entities = json["package_entities"].get<std::vector<std::string>>();

        if(json.contains("decoy_entities"))
            config.decoy_entities = json["decoy_entities"].get<std::vector<std::string>>();

        if(json.contains("weapon_entities"))
            config.weapon_entities = json["weapon_entities"].get<std::vector<std::string>>();

        if(json.contains("player_damage_sounds"))
            config.player_damage_sounds = json["player_damage_sounds"].get<std::vector<std::string>>();

        if(json.contains("player_death_sounds"))
            config.player_death_sounds = json["player_death_sounds"].get<std::vector<std::string>>();

        config.max_experience = json.value("max_experience", config.max_experience);

        if(json.contains("player_levels"))
            config.player_levels = json["player_levels"].get<std::vector<int>>();
    }
}
