
#pragma once

namespace game
{
    enum WeaponModifier : int
    {
        DAMAGE,
        SPREAD,
        CRIT_CHANCE,
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
