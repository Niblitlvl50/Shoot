
#pragma once

#include <cstdint>

namespace game
{
    enum PhysicsMaterial : uint32_t
    {
        UNSPECIFIED,
        ROCK,
        TREE,
        FLESH,

        NUM_MATERIALS
    };

    constexpr const char* g_physics_material_strings[] = {
        "Unspecified",
        "Rock",
        "Tree",
        "Flesh",
    };

    inline const char* PhysicsMaterialToString(PhysicsMaterial physics_material)
    {
        return g_physics_material_strings[(uint32_t)physics_material];
    };
}
