
#pragma once

namespace shared
{
    enum InteractionType : int
    {
        BUTTON,
        NPC,
        SIGN,
    };

    constexpr const char* interaction_type_strings[] = {
        "Button",
        "NPC",
        "Sign"
    };

    constexpr const char* interaction_type_verb[] = {
        "Push",
        "Talk",
        "Read",
    };

    inline const char* InteractionTypeToString(InteractionType interaction_type)
    {
        return interaction_type_strings[static_cast<int>(interaction_type)];
    }

    inline const char* InteractionTypeToVerb(InteractionType interaction_type)
    {
        return interaction_type_verb[static_cast<int>(interaction_type)];
    }
}
