
#include "WeaponTypes.h"
#include "System/Hash.h"



extern const game::WeaponSetup game::GENERIC            { hash::Hash("generic"),        hash::Hash("generic_bullet")        };
extern const game::WeaponSetup game::PLASMA_GUN         { hash::Hash("plasma_gun"),     hash::Hash("plasma_bullet")         };
extern const game::WeaponSetup game::PLASMA_GUN_HOMING  { hash::Hash("plasma_gun"),     hash::Hash("plasma_bullet_homing")  };
extern const game::WeaponSetup game::FLAK_CANON         { hash::Hash("flak_gun"),       hash::Hash("flak_bullet")           };
extern const game::WeaponSetup game::FLAK_CANON_PLASMA  { hash::Hash("flak_gun"),       hash::Hash("plasma_bullet")         };
extern const game::WeaponSetup game::CACO_PLASMA        { hash::Hash("cacoplasma"),     hash::Hash("caco_bullet")           };
extern const game::WeaponSetup game::ROCKET_LAUNCHER    { hash::Hash("rocketlauncher"), hash::Hash("rocket")                };
extern const game::WeaponSetup game::LASER_BLASTER      { hash::Hash("blaster"),        hash::Hash("laser")                 };
extern const game::WeaponSetup game::TURRET             { hash::Hash("generic"),        hash::Hash("generic_bullet")        };

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
