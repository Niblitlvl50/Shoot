
#pragma once

#include "ZoneCreationContext.h"
#include "ZoneFlow.h"
#include "MonoFwd.h"
#include "Engine.h"

#include <memory>

namespace game
{
    using IZonePtr = std::unique_ptr<mono::IZone>;

    class ZoneManager
    {
    public:

        ZoneManager(System::IWindow* window, mono::ICamera* camera, const ZoneCreationContext& zone_context, int initial_zone);
        void Run();

    private:

        mono::Engine m_engine;
        ZoneCreationContext m_zone_context;
        int m_active_zone;

        using LoadFunction = game::IZonePtr(*)(const ZoneCreationContext& zone_context);
        LoadFunction m_zones[N_ZONES];
    };
}
