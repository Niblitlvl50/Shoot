
#pragma once

#include "PerkTypes.h"
#include "System/Hash.h"

#include "nlohmann/json.hpp"

namespace game
{
    inline PerkType PerkTypeFromString(const std::string& str)
    {
        if(str == "IncreasedDamage")        return PerkType::IncreasedDamage;
        if(str == "IncreasedFireRate")       return PerkType::IncreasedFireRate;
        if(str == "IncreasedBulletVelocity") return PerkType::IncreasedBulletVelocity;
        if(str == "IncreasedBulletSpread")   return PerkType::IncreasedBulletSpread;
        if(str == "IncreasedBulletRange")    return PerkType::IncreasedBulletRange;
        if(str == "IncreasedMagazineSize")   return PerkType::IncreasedMagazineSize;
        if(str == "InfiniteAmmo")            return PerkType::InfiniteAmmo;
        if(str == "PiercingBullets")         return PerkType::PiercingBullets;
        if(str == "ExplosiveBullets")        return PerkType::ExplosiveBullets;
        if(str == "HomingBullets")           return PerkType::HomingBullets;
        if(str == "RicochetBullets")         return PerkType::RicochetBullets;
        if(str == "FireBullets")             return PerkType::FireBullets;
        if(str == "IceBullets")              return PerkType::IceBullets;
        if(str == "ElectricBullets")         return PerkType::ElectricBullets;
        if(str == "VampiricBullets")         return PerkType::VampiricBullets;
        if(str == "TimeSlowBullets")         return PerkType::TimeSlowBullets;
        return PerkType::None;
    }

    inline void from_json(const nlohmann::json& json, PerkSetup& perk_setup)
    {
        perk_setup.reroll_duration = json["reroll_duration"].get<float>();
    }

    inline void from_json(const nlohmann::json& json, PerkDefinition& perk_def)
    {
        perk_def.id             = json["id"].get<uint32_t>();
        perk_def.type           = PerkTypeFromString(json.value("type", std::string()));
        perk_def.name           = json["name"].get<std::string>();
        perk_def.description    = json["description"].get<std::string>();
        perk_def.icon_sprite_file = json["sprite_file"].get<std::string>();
    }
}
