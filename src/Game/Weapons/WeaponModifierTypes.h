
#pragma once

namespace game
{
    enum WeaponModifier : int
    {
        DAMAGE,
        SPREAD,
        CRIT_CHANCE,

        N_MODIFIERS
    };

    constexpr const char* g_weapon_modifier_to_sprite[] = {
        "res/sprites/powerup_damage_icon.sprite",
        "res/sprites/powerup_spread_icon.sprite",
        "res/sprites/powerup_crit_icon.sprite",
    };

    constexpr const char* g_weapon_modifier_type_names[] = {
        "Damage x2",
        "Bullet Wall",
        "Critical Hit +10",
    };

    inline const char* WeaponModifierTypeToString(WeaponModifier modifier_type)
    {
        return g_weapon_modifier_type_names[static_cast<int>(modifier_type)];
    }
}
