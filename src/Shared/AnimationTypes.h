
#pragma once

#include <cstdint>

namespace shared
{
    enum class AnimationType : uint32_t
    {
        DEFAULT,
        PING_PONG,
        NEVER_ENDING,
    };

    constexpr const char* animation_type_items[] = {
        "Default",
        "Ping Pong",
        "Never Ending",
    };

    inline const char* AnimationTypeToString(AnimationType animation_type)
    {
        return animation_type_items[static_cast<uint32_t>(animation_type)];
    }
}
