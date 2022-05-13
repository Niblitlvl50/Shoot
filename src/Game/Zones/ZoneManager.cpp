
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TitleScreen.h"
#include "SetupGameScreen.h"
#include "RemoteZone.h"
#include "SystemTestZone.h"
#include "EmptyZone.h"
#include "EndScreen.h"
#include "GameOverScreen.h"

#include <string>

using namespace game;

namespace
{
    template <typename T>
    game::IZonePtr LoadZone(const ZoneCreationContext& zone_context)
    {
        return std::make_unique<T>(zone_context);
    }
}

ZoneManager::ZoneManager(
    System::IWindow* window, mono::ICamera* camera, const ZoneCreationContext& zone_context)
    : m_engine(window, camera, zone_context.system_context, zone_context.event_handler)
    , m_zone_context(zone_context)
{
    m_zones[TITLE_SCREEN]       = LoadZone<game::TitleScreen>;
    m_zones[SETUP_GAME_SCREEN]  = LoadZone<game::SetupGameScreen>;
    m_zones[GAME_OVER_SCREEN]   = LoadZone<game::GameOverScreen>;
    m_zones[END_SCREEN]         = LoadZone<game::EndScreen>;
    m_zones[TEST_ZONE]          = LoadZone<game::WorldZone>;
    m_zones[TINY_ARENA_ZONE]    = LoadZone<game::TinyArenaZone>;
    m_zones[BOSS_ARENA_ZONE]    = LoadZone<game::BossArenaZone>;

    m_zones[EMPTY_ZONE]             = LoadZone<game::EmptyZone>;
    m_zones[ENEMY_TESTBED_ZONE]     = LoadZone<game::EnemyTestbedZone>;
    m_zones[TUTORIAL_DELIVERY_ZONE] = LoadZone<game::TutorialDeliveryZone>;

    m_zones[REMOTE_ZONE]        = LoadZone<game::RemoteZone>; // Not a GameZone
}

void ZoneManager::Run(int active_zone)
{
    while(true)
    {
        if(active_zone == QUIT)
            break;

        LoadFunction load_func = m_zones[active_zone];
        game::IZonePtr zone = load_func(m_zone_context);
        active_zone = m_engine.Run(zone.get());
    }
}
