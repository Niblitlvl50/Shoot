
#pragma once

#include "ZoneCreationContext.h"
#include "MonoPtrFwd.h"
#include "Engine.h"
#include <unordered_map>

namespace game
{
    class ZoneManager
    {
    public:

        ZoneManager(System::IWindow* window, const mono::ICameraPtr& camera, const ZoneCreationContext& zone_context);
        void Run();

    private:

        mono::Engine m_engine;
        ZoneCreationContext m_zone_context;

        using LoadFunction = mono::IZonePtr(*)(const ZoneCreationContext& zone_context);
        std::unordered_map<int, LoadFunction> m_zones;
    };
}
