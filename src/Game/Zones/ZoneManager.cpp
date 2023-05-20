
#include "ZoneManager.h"
#include "ZoneFlow.h"
#include "RemoteZone.h"
#include "SystemTestZone.h"

#include "Engine.h"
#include "System/Hash.h"
#include "System/Debug.h"
#include "System/File.h"
#include "System/System.h"

#include <string>
#include <unordered_map>
#include <vector>

#include "nlohmann/json.hpp"

using namespace game;

namespace game
{
    using IZonePtr = std::unique_ptr<mono::IZone>;
    using LoadFunction = game::IZonePtr(*)(const game::ZoneCreationContext& zone_context);

    struct Level
    {
        std::string name;
        std::string filename;
        std::string transitions[game::ZoneResult::ZR_COUNT];
    };

    struct LevelConfig
    {
        std::string start_level;
        std::vector<Level> levels;
    };

    inline void from_json(const nlohmann::json& json, game::Level& level)
    {
        level.name = json["name"];
        level.filename = json["filename"];

        const nlohmann::json& transitions_json = json["transitions"];
        level.transitions[game::ZoneResult::ZR_GAME_OVER] = transitions_json["gameover"];
        level.transitions[game::ZoneResult::ZR_COMPLETED] = transitions_json["completed"];
        level.transitions[game::ZoneResult::ZR_COMPLETED_ALT] = transitions_json["completed_alt"];
        level.transitions[game::ZoneResult::ZR_ABORTED] = transitions_json["aborted"];
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
        { "remote_network_zone",    LoadZone<game::RemoteZone>  },
    };
}

void ZoneManager::Run(mono::ICamera* camera, ZoneCreationContext zone_context, const char* initial_zone_name)
{
    mono::Engine engine(zone_context.window, camera, zone_context.system_context, zone_context.event_handler);
    LevelConfig level_config = LoadLevelConfig("res/configs/level_config.json");

    std::string zone_name =
        (initial_zone_name != nullptr) ? initial_zone_name : level_config.start_level;

    while(true)
    {
        if(zone_name == QUIT_NAME)
            break;

        const auto find_zone = [&zone_name](const game::Level& level) {
            return zone_name == level.name;
        };

        const auto level_it = std::find_if(level_config.levels.begin(), level_config.levels.end(), find_zone);
        if(level_it == level_config.levels.end())
        {
            System::Log("ZoneManager|Unable to find world with name '%s'", zone_name.c_str());
            break;
        }

        MONO_ASSERT(level_it != level_config.levels.end());

        zone_context.zone_filename = level_it->filename.c_str();

        const auto load_func_it = g_zone_load_func.find(zone_name);
        const LoadFunction load_func =
            (load_func_it != g_zone_load_func.end()) ? load_func_it->second : LoadZone<SystemTestZone>;

        game::IZonePtr zone = load_func(zone_context);
        const int zone_run_result = engine.Run(zone.get());
        MONO_ASSERT(zone_run_result >= 0 && zone_run_result < ZR_COUNT);

        zone_name = level_it->transitions[zone_run_result];
    }
}
