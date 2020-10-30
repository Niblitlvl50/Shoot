
#pragma once

#include <cstdint>
#include <vector>

namespace shared
{
    enum SpriteProperty : uint32_t
    {
        SP_NONE = 0,
        WIND_SWAY = 1,
        FLASH = 2,
    };

    static const std::vector<uint32_t> all_sprite_properties = {
        SpriteProperty::WIND_SWAY,
        SpriteProperty::FLASH,
    };

    inline const char* SpritePropertyToString(uint32_t category)
    {
        switch(category)
        {
        case SpriteProperty::SP_NONE:
            return "None";
        case SpriteProperty::WIND_SWAY:
            return "Wind Sway";
        case SpriteProperty::FLASH:
            return "Flash";
        };

        return "Unknown";
    };
}
