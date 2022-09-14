
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "TitleScreen.h"
#include "SetupGameScreen.h"
#include "RemoteZone.h"
#include "SystemTestZone.h"
#include "EndScreen.h"
#include "GameOverScreen.h"

#include "System/Hash.h"
#include "System/Debug.h"
#include "System/File.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"

using namespace game;

namespace game
{
    using IZonePtr = std::unique_ptr<mono::IZone>;
    using LoadFunction = game::IZonePtr(*)(const game::ZoneCreationContext& zone_context);

    inline void from_json(const nlohmann::json& json, game::Level& level)
    {
        level.name = json["name"];
        level.filename = json["filename"];
        level.transitions = json["transitions"];
    }

    LevelConfig LoadLevelConfig(const char* level_config_file)
    {
        const std::vector<byte> file_data = file::FileReadAll(level_config_file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        LevelConfig level_config;
        level_config.start_level = json["start_level"];
        level_config.levels = json["levels"];

        return level_config;
    }

    template <typename T>
    game::IZonePtr LoadZone(const ZoneCreationContext& zone_context)
    {
        return std::make_unique<T>(zone_context);
    }

    static const std::string QUIT_NAME              = "quit";
    static const std::string REMOTE_NETWORK_NAME    = "remote_network_zone";

    static const std::unordered_map<std::string, LoadFunction> g_zone_load_func = {
        { "title_screen",           LoadZone<game::TitleScreen>     },
        { "setup_game_screen",      LoadZone<game::SetupGameScreen> },
        { "gameover_screen",        LoadZone<game::GameOverScreen>  },
        { "end_screen",             LoadZone<game::EndScreen>       },
        { "remote_network_zone",    LoadZone<game::RemoteZone>      },
    };
}

ZoneManager::ZoneManager(
    System::IWindow* window, mono::ICamera* camera, const ZoneCreationContext& zone_context)
    : m_engine(window, camera, zone_context.system_context, zone_context.event_handler)
    , m_zone_context(zone_context)
{
    m_level_config = LoadLevelConfig("res/level_config.json");
}

void ZoneManager::Run(const char* initial_zone_name)
{
    std::string zone_name =
        (initial_zone_name != nullptr) ? initial_zone_name : m_level_config.start_level;

    while(true)
    {
        if(zone_name == QUIT_NAME)
            break;

        const auto find_zone = [&zone_name](const game::Level& level) {
            return zone_name == level.name;
        };

        const auto level_it = std::find_if(m_level_config.levels.begin(), m_level_config.levels.end(), find_zone);
        MONO_ASSERT(level_id != m_level_config.levels.end());

        m_zone_context.zone_filename = level_it->filename.c_str();

        const auto load_func_it = g_zone_load_func.find(zone_name);
        const LoadFunction load_func =
            (load_func_it != g_zone_load_func.end()) ? load_func_it->second : LoadZone<SystemTestZone>;

        game::IZonePtr zone = load_func(m_zone_context);
        const int zone_run_result = m_engine.Run(zone.get());
        MONO_ASSERT(zone_run_result >= 0 && zone_run_result < ZR_COUNT);

        zone_name = level_it->transitions[zone_run_result];
    }
}
