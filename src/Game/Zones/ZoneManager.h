
#pragma once

#include "ZoneCreationContext.h"
#include "ZoneFlow.h"
#include "MonoFwd.h"
#include "Engine.h"

#include <memory>

namespace game
{
    using IZonePtr = std::unique_ptr<mono::IZone>;
    using LoadFunction = game::IZonePtr(*)(const ZoneCreationContext& zone_context);

    class ZoneManager
    {
    public:

        ZoneManager(System::IWindow* window, mono::ICamera* camera, const ZoneCreationContext& zone_context);
        void Run(const char* zone_filename);

    private:

        mono::Engine m_engine;
        ZoneCreationContext m_zone_context;
    };
}
