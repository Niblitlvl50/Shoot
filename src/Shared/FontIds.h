
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
        PIXELETTE_MEGA,

        RUSSOONE_TINY,
        RUSSOONE_SMALL,
        RUSSOONE_MEDIUM,
        RUSSOONE_LARGE,
        RUSSOONE_MEGA,

        MONOTON_TINY,
        MONOTON_SMALL,
        MONOTON_MEDIUM,
        MONOTON_LARGE,
        MONOTON_MEGA,
    };

    constexpr const char* font_id_strings[] = {
        "PIXELETTE_TINY",
        "PIXELETTE_SMALL",
        "PIXELETTE_MEDIUM",
        "PIXELETTE_LARGE",
        "PIXELETTE_MEGA",

        "RUSSOONE_TINY",
        "RUSSOONE_SMALL",
        "RUSSOONE_MEDIUM",
        "RUSSOONE_LARGE",
        "RUSSOONE_MEGA",

        "MONOTON_TINY",
        "MONOTON_SMALL",
        "MONOTON_MEDIUM",
        "MONOTON_LARGE",
        "MONOTON_MEGA",
    };

    inline const char* FontIdToString(FontId font_id)
    {
        return font_id_strings[static_cast<uint32_t>(font_id)];
    }

    bool LoadFonts();
}
