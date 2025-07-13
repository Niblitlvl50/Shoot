
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class WeaponModifierFactory
    {
    public:

        static IWeaponModifier* CreateModifierForWeaponAndLevel(uint32_t weapon_identifier_hash, int level);
    };
}
