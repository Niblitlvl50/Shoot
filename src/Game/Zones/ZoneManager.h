
#pragma once

#include "MonoFwd.h"

#include "ZoneCreationContext.h"
#include "ZoneFlow.h"

#include <vector>
#include <string>

namespace game
{
    struct Level
    {
        std::string name;
        std::string filename;
        std::string transitions[game::ZoneResult::ZR_COUNT];
    };

    enum class LevelEntry
    {
        NEW_GAME,       // The first level since the game started.
        ZONE_RESULT,    // From the previous level's transitions, completed, game over and so on.
        LEVEL_SWITCH,   // From SwitchToLevel, a level_exit or the debug level menu.
    };

    struct LevelEntryInfo
    {
        LevelEntry entry = LevelEntry::NEW_GAME;
        std::string previous_level; // Empty for NEW_GAME.
    };

    class ZoneManager
    {
    public:

        static std::vector<Level> GetLevels();
        static void SwitchToLevel(const Level& level);

        // How the running level was reached.
        static const LevelEntryInfo& GetLevelEntryInfo();

        // Fades out the screen and then quits the running zone, the next level is the one passed to
        // SwitchToLevel or else picked from the zone's result. Every level fades in when it starts.
        static void ExitZone();

        static void Run(
            mono::ICamera* camera, ZoneCreationContext zone_context, const char* initial_zone_name = nullptr);
    };
}
