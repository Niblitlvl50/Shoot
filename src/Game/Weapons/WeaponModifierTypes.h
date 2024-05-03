
#pragma once

namespace game
{
    enum WeaponModifier
    {
        DAMAGE,
        SPREAD,

        N_MODIFIERS
    };

    constexpr const char* g_weapon_modifier_to_sprite[] = {
        "res/sprites/powerup_damage_icon.sprite",
        "res/sprites/powerup_spread_icon.sprite",
    };
}
