
#pragma once

#include <cstdint>

namespace shared
{
    enum class EntityLogicType : uint32_t
    {
        BAT,
        GOBLIN_FIRE,
        EYE_MONSTER,
        CACODEMON,
        FLYING_MONSTER,
        INVADER_PATH,
        BLOB,
        TURRET_SPAWNER,
        EXPLODABLE,
    };

    constexpr const char* entity_logic_strings[] = {
        "Bat",
        "Goblin Fire",
        "Eye Monster",
        "Cacodemon",
        "Flying Monster",
        "Path Invader",
        "Blob",
        "Turret Spawner",
        "Explodable",
    };

    inline const char* EntityLogicToString(EntityLogicType logic_type)
    {
        return entity_logic_strings[static_cast<uint32_t>(logic_type)];
    }
}
