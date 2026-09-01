
#pragma once

#include <string>
#include <vector>

namespace game
{
    struct PlayerConfig
    {
        std::vector<std::string> player_entities;
        std::vector<std::string> familiar_entities;
        std::vector<std::string> package_entities;
        std::vector<std::string> decoy_entities;
        std::vector<std::string> weapon_entities;

        std::vector<std::string> player_damage_sounds;
        std::vector<std::string> player_death_sounds;

        int max_experience = 1000;
        std::vector<int> player_levels;
    };

    bool LoadPlayerConfig(const char* config_file, PlayerConfig& config);
}
