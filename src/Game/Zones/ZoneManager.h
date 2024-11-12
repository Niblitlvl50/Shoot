
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

        static void Run(
            mono::ICamera* camera, ZoneCreationContext zone_context, const char* initial_zone_name = nullptr);
    };
}
