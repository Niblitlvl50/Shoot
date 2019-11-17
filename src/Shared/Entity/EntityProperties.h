
#pragma once

#include <cstdint>

enum EntityProperties : uint32_t
{
    REPLICATE = 1,
    RED_FLAG = 2,
    BLUE_FLAG = 4,
};

constexpr EntityProperties all_entity_properties[] = {
    EntityProperties::REPLICATE,
    EntityProperties::RED_FLAG,
    EntityProperties::BLUE_FLAG,
};

inline const char* EntityPropertyToString(EntityProperties property)
{
    switch(property)
    {
    case REPLICATE:
        return "Replicate";
    case RED_FLAG:
        return "Red Flag";
    case BLUE_FLAG:
        return "Blue Flag";
    }

    return "Unknown";
}
