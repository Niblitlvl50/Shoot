
#pragma once

namespace game
{
    enum InteractionType : int
    {
        BUTTON,
        NPC,
        SIGN,
        PICKUP,
        WEAPON,
    };

    constexpr const char* interaction_type_strings[] = {
        "Button",
        "NPC",
        "Sign",
        "Pickup",
        "Weapon",
    };

    constexpr const char* interaction_type_verb[] = {
        "Push",
        "Talk",
        "Read",
        "Pickup",
        "Take",
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
