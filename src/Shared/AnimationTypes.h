
#pragma once

#include <cstdint>

namespace shared
{
    enum class AnimationMode : uint32_t
    {
        DEFAULT,
        PING_PONG,
        NEVER_ENDING,
    };

    constexpr const char* animation_mode_items[] = {
        "Default",
        "Ping Pong",
        "Never Ending",
    };

    inline const char* AnimationModeToString(AnimationMode animation_mode)
    {
        return animation_mode_items[static_cast<uint32_t>(animation_mode)];
    }
}
