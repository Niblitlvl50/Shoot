
#pragma once

#include <cstdint>

namespace game
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

        UNUSED_TINY,
        UNUSED_SMALL,
        UNUSED_MEDIUM,
        UNUSED_LARGE,
        UNUSED_MEGA,

        BLACK_HAN_SANS_TINY,
        BLACK_HAN_SANS_SMALL,
        BLACK_HAN_SANS_MEDIUM,
        BLACK_HAN_SANS_LARGE,
        BLACK_HAN_SANS_MEGA,
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

        "UNUSED_TINY",
        "UNUSED_SMALL",
        "UNUSED_MEDIUM",
        "UNUSED_LARGE",
        "UNUSED_MEGA",

        "BLACK_HAN_SANS_TINY",
        "BLACK_HAN_SANS_SMALL",
        "BLACK_HAN_SANS_MEDIUM",
        "BLACK_HAN_SANS_LARGE",
        "BLACK_HAN_SANS_MEGA",
    };

    inline const char* FontIdToString(FontId font_id)
    {
        return font_id_strings[static_cast<uint32_t>(font_id)];
    }

    bool LoadFonts();
}
