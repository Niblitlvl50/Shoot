
#pragma once

#include <vector>
#include <cstdint>

enum EntityProperties : uint32_t
{
    REPLICATE = 1,
    CTF_FLAG = 2,
};

static const std::vector<uint32_t> all_entity_properties = {
    EntityProperties::REPLICATE,
    EntityProperties::CTF_FLAG,
};

inline const char* EntityPropertyToString(uint32_t property)
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
