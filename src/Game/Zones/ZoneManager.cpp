
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TitleScreen.h"
#include "SetupGameScreen.h"
#include "RemoteZone.h"
#include "SystemTestZone.h"
#include "EmptyZone.h"
#include "EndScreen.h"
#include "GameOverScreen.h"
#include "SystemContext.h"


#include "System/System.h"
#include "System/Hash.h"

#include <string>
#include <unordered_map>
#include <cassert>

using namespace game;

namespace
{
    template <typename T>
    game::IZonePtr LoadZone(const ZoneCreationContext& zone_context)
    {
        return std::make_unique<T>(zone_context);
    }

    static const uint32_t QUIT_HASH                 = hash::Hash("quit");
    static const uint32_t TITLE_SCREEN_HASH         = hash::Hash("res/worlds/title_screen.components");
    static const uint32_t SETUP_GAME_SCREEN_HASH    = hash::Hash("res/worlds/setup_game_screen.components");
    static const uint32_t GAME_OVER_SCREEN_HASH     = hash::Hash("res/worlds/game_over.components");
    static const uint32_t END_SCREEN_HASH           = hash::Hash("res/worlds/end_screen.components");
    static const uint32_t WORLD_ARENA_HASH          = hash::Hash("res/worlds/world.components");
    static const uint32_t TINY_ARENA_HASH           = hash::Hash("res/worlds/tiny_arena.components");
    static const uint32_t BOSS_ARENA_HASH           = hash::Hash("res/worlds/boss_arena.components");
    static const uint32_t EMPTY_ARENA_HASH          = hash::Hash("res/worlds/empty_world.components");
    static const uint32_t ENEMY_TESTBED_HASH        = hash::Hash("res/worlds/enemy_testbed.components");
    static const uint32_t TUTORIAL_DELIVERY_HASH    = hash::Hash("res/worlds/tutorial_delivery.components");
    static const uint32_t REMOTE_NETWORK_HASH       = hash::Hash("remote_network_zone");

    static const std::unordered_map<uint32_t, LoadFunction> g_zone_load_func = {
        { TITLE_SCREEN_HASH,        LoadZone<game::TitleScreen> },
        { SETUP_GAME_SCREEN_HASH,   LoadZone<game::SetupGameScreen> },
        { GAME_OVER_SCREEN_HASH,    LoadZone<game::GameOverScreen> },
        { END_SCREEN_HASH,          LoadZone<game::EndScreen> },
        { WORLD_ARENA_HASH,         LoadZone<game::SystemTestZone> },
        { TINY_ARENA_HASH,          LoadZone<game::SystemTestZone> },
        { BOSS_ARENA_HASH,          LoadZone<game::SystemTestZone> },
        { EMPTY_ARENA_HASH,         LoadZone<game::EmptyZone> },
        { ENEMY_TESTBED_HASH,       LoadZone<game::SystemTestZone> },
        { TUTORIAL_DELIVERY_HASH,   LoadZone<game::SystemTestZone> },
        { REMOTE_NETWORK_HASH,      LoadZone<game::RemoteZone> },
    };

    struct ZoneTransition
    {
        uint32_t zone[ZR_COUNT];
    };

    static const std::unordered_map<uint32_t, ZoneTransition> g_zone_transitions = {

        // Screens
        { TITLE_SCREEN_HASH,        { QUIT_HASH,                SETUP_GAME_SCREEN_HASH, QUIT_HASH } },
        { GAME_OVER_SCREEN_HASH,    { GAME_OVER_SCREEN_HASH,    TITLE_SCREEN_HASH,      TITLE_SCREEN_HASH } },
        { END_SCREEN_HASH,          { GAME_OVER_SCREEN_HASH,    TITLE_SCREEN_HASH,      TITLE_SCREEN_HASH } },
        { SETUP_GAME_SCREEN_HASH,   { GAME_OVER_SCREEN_HASH,    TINY_ARENA_HASH,        TITLE_SCREEN_HASH } },
        
        // Tutorials
        { TUTORIAL_DELIVERY_HASH,   { GAME_OVER_SCREEN_HASH,    TINY_ARENA_HASH,        TITLE_SCREEN_HASH } },

        // Arenas
        { WORLD_ARENA_HASH,         { GAME_OVER_SCREEN_HASH,    END_SCREEN_HASH,        TITLE_SCREEN_HASH } },
        { TINY_ARENA_HASH,          { GAME_OVER_SCREEN_HASH,    BOSS_ARENA_HASH,        TITLE_SCREEN_HASH } },
        { BOSS_ARENA_HASH,          { GAME_OVER_SCREEN_HASH,    END_SCREEN_HASH,        TITLE_SCREEN_HASH } },

        // Testing
        { EMPTY_ARENA_HASH,         { GAME_OVER_SCREEN_HASH,    END_SCREEN_HASH,        TITLE_SCREEN_HASH } },
        { ENEMY_TESTBED_HASH,       { GAME_OVER_SCREEN_HASH,    END_SCREEN_HASH,        TITLE_SCREEN_HASH } },

        // Special
        { REMOTE_NETWORK_HASH,      { GAME_OVER_SCREEN_HASH,    END_SCREEN_HASH,        TITLE_SCREEN_HASH } },
    };

    std::unordered_map<uint32_t, std::string> g_zone_names = {
        { TITLE_SCREEN_HASH,        "res/worlds/title_screen.components" },
        { SETUP_GAME_SCREEN_HASH,   "res/worlds/setup_game_screen.components" },
        { GAME_OVER_SCREEN_HASH,    "res/worlds/game_over.components" },
        { END_SCREEN_HASH,          "res/worlds/end_screen.components" },
        { WORLD_ARENA_HASH,         "res/worlds/world.components" },
        { TINY_ARENA_HASH,          "res/worlds/tiny_arena.components" },
        { BOSS_ARENA_HASH,          "res/worlds/boss_arena.components" },
        { EMPTY_ARENA_HASH,         "res/worlds/empty_world.components" },
        { ENEMY_TESTBED_HASH,       "res/worlds/enemy_testbed.components" },
        { TUTORIAL_DELIVERY_HASH,   "res/worlds/tutorial_delivery.components" },
        { REMOTE_NETWORK_HASH,      "remote_network_zone" },
    };
}


ZoneManager::ZoneManager(
    System::IWindow* window, mono::ICamera* camera, const ZoneCreationContext& zone_context)
    : m_engine(window, camera, zone_context.system_context, zone_context.event_handler)
    , m_zone_context(zone_context)
{
}

void ZoneManager::Run(const char* initial_zone_filename)
{
    uint32_t zone_hash = (initial_zone_filename != nullptr) ? hash::Hash(initial_zone_filename) : TITLE_SCREEN_HASH;

    while(true)
    {
        if(zone_hash == QUIT_HASH)
            break;

        const LoadFunction load_func = g_zone_load_func.find(zone_hash)->second;
        const ZoneTransition zone_transition = g_zone_transitions.find(zone_hash)->second;
        const std::string zone_filename = g_zone_names.find(zone_hash)->second;

        m_zone_context.zone_filename = zone_filename.c_str();

        game::IZonePtr zone = load_func(m_zone_context);
        const int zone_run_result = m_engine.Run(zone.get());
        assert(zone_run_result >= 0 && zone_run_result < ZR_COUNT);
        zone_hash = zone_transition.zone[zone_run_result];

        m_zone_context.system_context->Reset();
    }
}
