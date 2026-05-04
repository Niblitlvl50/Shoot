
#pragma once

#include "PerkTypes.h"
#include "System/Hash.h"

#include "nlohmann/json.hpp"

namespace game
{
    inline void from_json(const nlohmann::json& json, PerkDefinition& weapon_config)
    {
        weapon_config.id                    = json["id"].get<uint32_t>();
        weapon_config.name                  = json["name"].get<std::string>();
        weapon_config.description           = json["description"].get<std::string>();
        weapon_config.icon_sprite_id        = hash::Hash(json["icon"].get<std::string>().c_str());
    }
}
