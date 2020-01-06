
#pragma once

#include <cstdint>

enum EntityProperties : uint32_t
{
    REPLICATE = 1,
    CTF_FLAG = 2,
};

constexpr EntityProperties all_entity_properties[] = {
    EntityProperties::REPLICATE,
    EntityProperties::CTF_FLAG,
};

inline const char* EntityPropertyToString(EntityProperties property)
{
    switch(property)
    {
    case REPLICATE:
        return "Replicate";
    case CTF_FLAG:
        return "CTF Flag";
    }

    return "Unknown";
}
