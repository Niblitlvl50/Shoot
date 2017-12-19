
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TestZone.h"
#include "TitleScreen.h"

using namespace game;

namespace
{
    template <typename T>
    mono::IZonePtr LoadZone(mono::EventHandler& event_handler)
    {
        return std::make_shared<T>(event_handler);
    }
}

ZoneManager::ZoneManager(System::IWindow* window, const mono::ICameraPtr& camera, mono::EventHandler& event_handler)
    : m_engine(window, camera, event_handler)
    , m_event_handler(event_handler)
{
    m_zones[TITLE_SCREEN] = LoadZone<game::TitleScreen>;
    m_zones[TEST_ZONE] = LoadZone<game::TestZone>;
}

void ZoneManager::Run()
{
    int active_zone = TITLE_SCREEN;

    while(true)
    {
        mono::IZonePtr zone = m_zones[active_zone](m_event_handler);
        const int exit_code = m_engine.Run(zone);
        if(exit_code == QUIT)
            break;

        active_zone = exit_code;
    }
}
