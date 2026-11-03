
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

    class ZoneManager
    {
    public:

        static std::vector<Level> GetLevels();
        static void SwitchToLevel(const Level& level);

        // Fades out the screen and then quits the running zone, the next level is the one passed to
        // SwitchToLevel or else picked from the zone's result. Every level fades in when it starts.
        static void ExitZone();

        static void Run(
            mono::ICamera* camera, ZoneCreationContext zone_context, const char* initial_zone_name = nullptr);
    };
}
