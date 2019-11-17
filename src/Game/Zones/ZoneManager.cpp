
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TitleScreen.h"
#include "RemoteZone.h"
#include "SystemTestZone.h"

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
    System::IWindow* window, const mono::ICameraPtr& camera, const ZoneCreationContext& zone_context, int initial_zone)
    : m_engine(window, camera, zone_context.system_context, zone_context.event_handler)
    , m_zone_context(zone_context)
    , m_active_zone(initial_zone)
{
    m_zones[TITLE_SCREEN] = LoadZone<game::TitleScreen>;
    m_zones[TEST_ZONE] = LoadZone<game::SystemTestZone>;
    m_zones[REMOTE_ZONE] = LoadZone<game::RemoteZone>;
}

void ZoneManager::Run()
{
    while(true)
    {
        if(m_active_zone == QUIT)
            break;

        LoadFunction load_func = m_zones[m_active_zone];
        mono::IZonePtr zone = load_func(m_zone_context);
        m_active_zone = m_engine.Run(zone.get());
    }
}
