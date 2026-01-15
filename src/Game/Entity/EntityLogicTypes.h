
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
        BIRD,
        EXPLODABLE,
        FLAMING_SKULL_BOSS,
        IMP,
        BOMB_THROWER,
        DEMON_MINION_BOSS,
        REACTIVE_PROP,
        GOLEM_TINY,
    };

    constexpr const char* g_entity_logic_strings[] = {
        "Bat",
        "Goblin Fire",
        "Eye Monster",
        "Cacodemon",
        "Flying Monster",
        "Path Invader",
        "Blob",
        "Bird",
        "Explodable",
        "Flaming Skull Boss",
        "Imp",
        "Bomb Thrower",
        "Demon Minion Boss",
        "Reactive Prop",
        "Golem Tiny",
    };

    inline const char* EntityLogicToString(EntityLogicType logic_type)
    {
        return g_entity_logic_strings[static_cast<uint32_t>(logic_type)];
    }
}
