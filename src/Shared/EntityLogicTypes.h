
#pragma once

#include <cstdint>

enum class EntityLogicType : uint32_t
{
    BAT,
    BEAST,
    BLACK_SQUARE,
    CACODEMON,
    INVADER,
    INVADER_PATH
};

constexpr const char* entity_logic_strings[] = {
    "Bat",
    "Beast",
    "Black Square",
    "Cacodemon",
    "Invader",
    "Path Invader"
};

inline const char* EntityLogicToString(EntityLogicType logic_type)
{
    return entity_logic_strings[static_cast<uint32_t>(logic_type)];
}

