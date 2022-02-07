
#pragma once

#include "Rendering/Color.h"

namespace game
{
    enum PlayerAbility
    {
        WEAPON_RELOAD,
        BLINK,
        SHOCKWAVE,

        N_ABILITIES
    };

    constexpr mono::Color::RGBA g_ability_to_color[] = {
        mono::Color::BLACK,
        mono::Color::RED,
        mono::Color::GREEN
    };
}
