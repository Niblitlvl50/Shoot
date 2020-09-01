
#pragma once

#include <cstdint>

namespace shared
{
    enum FontId : uint32_t
    {
        PIXELETTE_TINY,
        PIXELETTE_SMALL,
        PIXELETTE_MEDIUM,
        PIXELETTE_LARGE,
        PIXELETTE_MEGA
    };

    constexpr const char* font_id_strings[] = {
        "PIXELETTE_TINY",
        "PIXELETTE_SMALL",
        "PIXELETTE_MEDIUM",
        "PIXELETTE_LARGE",
        "PIXELETTE_MEGA"
    };

    inline const char* FontIdToString(FontId font_id)
    {
        return font_id_strings[static_cast<uint32_t>(font_id)];
    }
}
