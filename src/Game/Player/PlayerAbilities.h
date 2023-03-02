
#pragma once

namespace game
{
    enum PlayerAbility
    {
        WEAPON_RELOAD,
        BLINK,
        SHOCKWAVE,
        SHIELD,
        WEAPON_AMMUNITION,

        N_ABILITIES
    };

    constexpr const char* g_ability_to_sprite[] = {
        "res/sprites/ammo_icon.sprite",
        "res/sprites/energy_icon.sprite",
        "res/sprites/energy_icon.sprite",
        "res/sprites/energy_icon.sprite",
        "res/sprites/ammo_icon.sprite",
    };

    constexpr const char* g_player_crosshair_sprite = "res/sprites/crosshair006.sprite";
}
