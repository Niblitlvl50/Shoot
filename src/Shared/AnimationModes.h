
#pragma once

#include <cstdint>
#include <vector>

namespace shared
{
    enum AnimationMode : uint32_t
    {
        TRIGGER_ACTIVATED = 1,
        PING_PONG = 2,
    };

    static const std::vector<uint32_t> all_animation_modes = {
        AnimationMode::TRIGGER_ACTIVATED,
        AnimationMode::PING_PONG,
    };

    inline const char* AnimationModeToString(uint32_t animation_mode)
    {
        switch(animation_mode)
        {
        case TRIGGER_ACTIVATED:
            return "Trigger Activated";
        case PING_PONG:
            return "Ping Pong";
        };

        return "Unknown";
    }
}
