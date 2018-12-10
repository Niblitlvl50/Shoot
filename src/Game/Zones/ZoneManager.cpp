
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TestZone.h"
#include "TitleScreen.h"

using namespace game;

namespace
{
    template <typename T>
    mono::IZonePtr LoadZone(const ZoneCreationContext& zone_context)
    {
        return std::make_shared<T>(zone_context);
    }
}

ZoneManager::ZoneManager(
    System::IWindow* window, const mono::ICameraPtr& camera, const ZoneCreationContext& zone_context)
    : m_engine(window, camera, 32.0f, *zone_context.event_handler)
    , m_zone_context(zone_context)
{
    m_zones[TITLE_SCREEN] = LoadZone<game::TitleScreen>;
    m_zones[TEST_ZONE] = LoadZone<game::TestZone>;
}

void ZoneManager::Run()
{
    int active_zone = TITLE_SCREEN;

    while(true)
    {
        mono::IZonePtr zone = m_zones[active_zone](m_zone_context);
        const int exit_code = m_engine.Run(zone);
        if(exit_code == QUIT)
            break;

        active_zone = exit_code;
    }
}
