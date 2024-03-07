
#pragma once

#include <memory>

namespace game
{
    enum class EnemyTargetMode : uint32_t
    {
        Normal,
        Horde
    };

    enum class TargetFaction : uint32_t
    {
        Player,
        Enemies
    };

    constexpr const char* g_target_faction_strings[] = {
        "Player",
        "Enemies",
    };

    inline const char* TargetFactionToString(TargetFaction property)
    {
        switch(property)
        {
        case TargetFaction::Player:
            return "Player";
        case TargetFaction::Enemies:
            return "Enemies";
        }

        return "Unknown";
    }

    using ITargetPtr = std::shared_ptr<class ITarget>;
}
