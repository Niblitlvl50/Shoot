
#pragma once

#include <cstdint>
#include <vector>

namespace editor
{
    enum EntityAttribute : uint32_t
    {
        LOCKED = 1
    };

    static const std::vector<uint32_t> all_entity_attributes = {
        EntityAttribute::LOCKED,
    };

    inline const char* EntityAttributeToString(uint32_t entity_attribute)
    {
        switch(entity_attribute)
        {
        case EntityAttribute::LOCKED:
            return "Locked";
        };

        return "Unknown";
    };
}
