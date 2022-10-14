
#pragma once

#include "ZoneCreationContext.h"
#include "ZoneFlow.h"
#include "MonoFwd.h"
#include "Engine.h"

#include <memory>
#include <string>
#include <vector>

namespace game
{
    struct Level
    {
        std::string name;
        std::string filename;
        std::vector<std::string> transitions;
    };

    struct LevelConfig
    {
        std::string start_level;
        std::vector<Level> levels;
    };

    class ZoneManager
    {
    public:

        ZoneManager(mono::ICamera* camera, const ZoneCreationContext& zone_context);
        void Run(const char* initial_zone_name = nullptr);

    private:

        mono::Engine m_engine;
        ZoneCreationContext m_zone_context;
        LevelConfig m_level_config;
    };
}
