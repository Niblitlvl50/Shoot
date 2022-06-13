
#pragma once

#include <cstdint>
#include <vector>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    enum AnimationMode : uint32_t
    {
        TRIGGER_ACTIVATED   = ENUM_BIT(0),
        PING_PONG           = ENUM_BIT(1),
        TRIGGER_REVERSE     = ENUM_BIT(2),
        ONE_SHOT            = ENUM_BIT(3),
        LOOPING             = ENUM_BIT(4),
    };

    static const std::vector<uint32_t> all_animation_modes = {
        AnimationMode::TRIGGER_ACTIVATED,
        AnimationMode::PING_PONG,
        AnimationMode::TRIGGER_REVERSE,
        AnimationMode::ONE_SHOT,
        AnimationMode::LOOPING,
    };

    inline const char* AnimationModeToString(uint32_t animation_mode)
    {
        switch(animation_mode)
        {
        case TRIGGER_ACTIVATED:
            return "Trigger Activated";
        case PING_PONG:
            return "Ping Pong";
        case TRIGGER_REVERSE:
            return "Trigger Reverse";
        case ONE_SHOT:
            return "One Shot";
        case LOOPING:
            return "Looping";
        };

        return "Unknown";
    }
}
