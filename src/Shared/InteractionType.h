
#pragma once

namespace shared
{
    enum InteractionType : int
    {
        BUTTON,
        NPC
    };

    constexpr const char* interaction_type_strings[] = {
        "Button",
        "NPC",
    };

    inline const char* InteractionTypeToString(InteractionType interaction_type)
    {
        return interaction_type_strings[static_cast<int>(interaction_type)];
    }
}
