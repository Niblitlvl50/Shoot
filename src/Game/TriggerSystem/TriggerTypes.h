
#pragma once

#include <cstdint>

namespace game
{
    enum class AreaTriggerOperation : int
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
        return area_trigger_op_strings[static_cast<int>(trigger_op)];
    }

    enum class DestroyedTriggerType : int
    {
        ON_DEATH,
        ON_DESTORYED
    };

    constexpr const char* destroyed_trigger_type_strings[] = {
        "On Death",
        "On Destroyed"
    };

    inline const char* DestroyedTriggerTypeToString(DestroyedTriggerType trigger_type)
    {
        return destroyed_trigger_type_strings[static_cast<int>(trigger_type)];
    }
}
