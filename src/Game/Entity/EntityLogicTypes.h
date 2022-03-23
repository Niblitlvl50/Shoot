
#pragma once

#include <cstdint>

namespace game
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
        FLAMING_SKULL_BOSS
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
        "Flaming Skull Boss"
    };

    inline const char* EntityLogicToString(EntityLogicType logic_type)
    {
        return entity_logic_strings[static_cast<uint32_t>(logic_type)];
    }
}
