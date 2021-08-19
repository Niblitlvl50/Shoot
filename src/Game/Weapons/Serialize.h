
#pragma once

#include "WeaponConfiguration.h"
#include "nlohmann/json.hpp"

namespace game
{
    // Vector
    inline void to_json(nlohmann::json& j, const BulletConfiguration& bullet_config)
    {
        j = nlohmann::json{
            { "life_span",       bullet_config.life_span },
            { "fuzzy_life_span", bullet_config.fuzzy_life_span },
            { "entity_file",     bullet_config.entity_file },
            { "sound_file",      bullet_config.sound_file },
            { "behaviour",       "" }
        };
    }

    inline void from_json(const nlohmann::json& json, BulletConfiguration& bullet_config)
    {
        bullet_config.life_span = json["life_span"].get<float>();
        bullet_config.fuzzy_life_span = json["fuzzy_life_span"].get<float>();
        bullet_config.entity_file = json["entity_file"].get<std::string>();
        bullet_config.sound_file = json["sound_file"].get<std::string>();

        bullet_config.bullet_behaviour;

        //BulletCollisionBehaviour bullet_behaviour = BulletCollisionBehaviour::NORMAL;
    }
}
