
#pragma once

#include <cstdint>

namespace shared
{
    enum class AreaTriggerOperation : uint32_t
    {
        EQUAL_TO,
        LESS_THAN,
        GREATER_THAN,
    };

    constexpr const char* area_trigger_op_strings[] = {
        "Equal (==)",
        "Less (<)",
        "Greater (>)",
    };

    inline const char* AreaTriggerOpToString(AreaTriggerOperation trigger_op)
    {
        return area_trigger_op_strings[static_cast<uint32_t>(trigger_op)];
    }
}
