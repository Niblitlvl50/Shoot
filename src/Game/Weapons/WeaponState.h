
#pragma once

namespace game
{
    enum class WeaponState
    {
        IDLE,
        FIRE,
        RELOADING,
        OUT_OF_AMMO
    };

    inline const char* WeaponStateToString(WeaponState state)
    {
        switch(state)
        {
        case WeaponState::IDLE:
            return "Idle";
        case WeaponState::FIRE:
            return "Fire";
        case WeaponState::RELOADING:
            return "Reloading";
        case WeaponState::OUT_OF_AMMO:
            return "Out of Ammo";
        }

        return "Unknown";
    }
}
