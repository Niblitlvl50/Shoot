
#pragma once

#include <cstdint>

enum EntityProperties : uint32_t
{
    REPLICATE = 1,
    SOMETHING_ELSE = 2,
    BAD_ASS = 4
};

constexpr EntityProperties all_entity_properties[] = {
    EntityProperties::REPLICATE,
    EntityProperties::SOMETHING_ELSE,
    EntityProperties::BAD_ASS
};

inline const char* EntityPropertyToString(EntityProperties property)
{
    switch(property)
    {
    case REPLICATE:
        return "Replicate";
    case SOMETHING_ELSE:
        return "Something else";
    case BAD_ASS:
        return "Is Bad Ass";
    }

    return nullptr;
}
