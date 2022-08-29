
#include "WeaponTypes.h"
#include "Weapons/Serialize.h"

#include "System/Hash.h"
#include "System/File.h"

#include "nlohmann/json.hpp"

extern const game::WeaponSetup game::NO_WEAPON { 0, 0 };

extern const game::WeaponSetup game::GENERIC            { hash::Hash("generic"),            hash::Hash("generic_bullet")        };
extern const game::WeaponSetup game::PLASMA_GUN         { hash::Hash("plasma_gun"),         hash::Hash("plasma_bullet")         };
extern const game::WeaponSetup game::FLAK_CANON         { hash::Hash("flak_gun"),           hash::Hash("flak_bullet")           };
extern const game::WeaponSetup game::FLAK_CANON_PLASMA  { hash::Hash("flak_gun"),           hash::Hash("plasma_bullet")         };
extern const game::WeaponSetup game::CACO_PLASMA_HOMING { hash::Hash("cacodemon_plasma"),   hash::Hash("caco_bullet_homing")    };
extern const game::WeaponSetup game::CACO_PLASMA        { hash::Hash("cacoplasma"),         hash::Hash("caco_bullet")           };
extern const game::WeaponSetup game::ROCKET_LAUNCHER    { hash::Hash("rocketlauncher"),     hash::Hash("rocket")                };
extern const game::WeaponSetup game::LASER_BLASTER      { hash::Hash("blaster"),            hash::Hash("laser")                 };
extern const game::WeaponSetup game::TURRET             { hash::Hash("generic"),            hash::Hash("generic_bullet")        };

std::vector<game::WeaponSetup> game::GetWeaponList()
{
    return {
        GENERIC,
        PLASMA_GUN,
        FLAK_CANON,
        FLAK_CANON_PLASMA,
        CACO_PLASMA,
        ROCKET_LAUNCHER,
        LASER_BLASTER
    };
}

std::vector<game::WeaponSetup> game::GetSupportWeaponList()
{
    return {
        TURRET,
    };
}

const char* game::GetWeaponNameFromHash(uint32_t weapon_hash)
{
    if(weapon_hash == NO_WEAPON.weapon_hash)
        return "None";
    else if(weapon_hash == GENERIC.weapon_hash)
        return "Bolter";
    else if(weapon_hash == PLASMA_GUN.weapon_hash)
        return "Plasma Gun";
    else if(weapon_hash == FLAK_CANON.weapon_hash)
        return "Flak Canon";
    else if(weapon_hash == FLAK_CANON_PLASMA.weapon_hash)
        return "Plasma Canon";
    else if(weapon_hash == CACO_PLASMA.weapon_hash)
        return "Caco Plasma";
    else if(weapon_hash == ROCKET_LAUNCHER.weapon_hash)
        return "Rocker Launcher";
    else if(weapon_hash == LASER_BLASTER.weapon_hash)
        return "Laser Blaster";
    else if(weapon_hash == TURRET.weapon_hash)
        return "turret";

    return "Unknown";
}

const char* game::GetWeaponSpriteFromHash(uint32_t weapon_hash)
{
    if(weapon_hash == GENERIC.weapon_hash)
        return "res/sprites/bolter.sprite";
    else if(weapon_hash == PLASMA_GUN.weapon_hash)
        return "res/sprites/sg2.sprite";
    else if(weapon_hash == FLAK_CANON.weapon_hash)
        return "res/sprites/flak_cannon.sprite";
    else if(weapon_hash == FLAK_CANON_PLASMA.weapon_hash)
        return "res/sprites/flak_cannon.sprite";
    else if(weapon_hash == CACO_PLASMA.weapon_hash)
        return "res/sprites/pow9.sprite";
    else if(weapon_hash == ROCKET_LAUNCHER.weapon_hash)
        return "res/sprites/rocket_launcher.sprite";
    else if(weapon_hash == LASER_BLASTER.weapon_hash)
        return "res/sprites/bolter.sprite";
    else if(weapon_hash == TURRET.weapon_hash)
        return "res/sprites/rocket_launcher.sprite";

    return "res/sprites/bolter.sprite";
}


game::WeaponConfig game::LoadWeaponConfig(const char* weapon_config_file)
{
    WeaponConfig weapon_config;

    file::FilePtr file = file::OpenAsciiFile(weapon_config_file);
    if(!file)
        return weapon_config;

    const std::vector<byte> file_data = file::FileRead(file);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    for(const BulletConfiguration bullet : json["bullets"])
        weapon_config.bullet_configs[hash::Hash(bullet.name.c_str())] =  bullet;

    for(const WeaponConfiguration weapon : json["weapons"])
        weapon_config.weapon_configs[hash::Hash(weapon.name.c_str())] =  weapon;

    for(const WeaponBulletCombination weapon_bullet_combo : json["weapon_setups"])
    {
        weapon_config.weapon_names.push_back(weapon_bullet_combo.name);
        weapon_config.weapon_combinations[hash::Hash(weapon_bullet_combo.name.c_str())] = weapon_bullet_combo;
    }

    return weapon_config;
}