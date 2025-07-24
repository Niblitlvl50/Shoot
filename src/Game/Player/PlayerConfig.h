
#pragma once

#include <string>
#include <vector>

namespace game
{
    struct PlayerConfig
    {
        std::string decoy_entity;
        std::string weapon_entity;
        std::vector<int> player_levels;
    };
}
