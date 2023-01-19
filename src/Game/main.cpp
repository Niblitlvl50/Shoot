
#include "Camera/Camera.h"
#include "EventHandler/EventHandler.h"
#include "SystemContext.h"

#include "System/Audio.h"
#include "System/System.h"
#include "System/Network.h"

#include "EntitySystem/IEntityManager.h"
#include "Rendering/RenderSystem.h"

#include "Player/PlayerInfo.h"
#include "FontIds.h"
#include "GameConfig.h"
#include "GameSystems.h"
#include "Resources.h"
#include "Zones/ZoneManager.h"

#include "Network/NetworkMessage.h"

#include "Entity/ComponentFunctions.h"
#include "Entity/GameComponentFuncs.h"

#include <cassert>
#include <cstring>

namespace
{
    struct Options
    {
        int x = 100;
        int y = 100;
        int width = 1200;
        int height = 750;
        const char* start_zone = nullptr;
        const char* game_config = "res/game_config.json";
        const char* log_file = "game_log.log";
    };

    Options ParseCommandline(int argc, char* argv[])
    {
        Options options;

        for(int index = 0; index < argc; ++index)
        {
            const char* arg = argv[index];
            if(std::strcmp(arg, "-position") == 0)
            {
                options.x = atoi(argv[++index]);
                options.y = atoi(argv[++index]);
            }
            else if(std::strcmp(arg, "-size") == 0)
            {
                options.width = atoi(argv[++index]);
                options.height = atoi(argv[++index]);
            }
            else if(std::strcmp(arg, "-zone") == 0)
            {
                options.start_zone = argv[++index];
            }
            else if(std::strcmp(arg, "-config") == 0)
            {
                options.game_config = argv[++index];
            }
            else if(std::strcmp(arg, "-log-file") == 0)
            {
                options.log_file = argv[++index];
            }
        }

        return options;
    }
}

int main(int argc, char* argv[])
{
    constexpr uint32_t max_entities = 1000;
    const Options options = ParseCommandline(argc, argv);

    System::InitializeContext system_init_context;
    system_init_context.log_file = options.log_file;

#ifdef __APPLE__
    char application_path_buffer[1024] = {};
    System::GetApplicationPath(application_path_buffer, std::size(application_path_buffer));
    system_init_context.working_directory = application_path_buffer;
#endif

    System::Initialize(system_init_context);

    game::Config game_config;
    game::LoadConfig(options.game_config, game_config);

    System::InitializeUserPath(game_config.organization.c_str(), game_config.application.c_str());

    game::LoadAllSprites("res/sprites/all_sprite_files.json");
    game::LoadAllTextures("res/textures/all_textures.json");
    game::LoadAllWorlds("res/worlds/all_worlds.json");

    network::Initialize(game_config.port_range_start, game_config.port_range_end);
    game::PrintNetworkMessageSize();
    game::InitializePlayerInfo();

    {
        const System::Size window_size = System::GetCurrentWindowSize();
        const float window_ratio = float(window_size.width) / float(window_size.height);
        const int height = float(options.width) / window_ratio;

        const int window_options = 
            0;
            //System::WindowOptions::FULLSCREEN;
            //System::WindowOptions::FULLSCREEN_DESKTOP;
            //System::WindowOptions::DISABLE_VSYNC;
        std::unique_ptr<System::IWindow> window(System::MakeWindow(
            game_config.application.c_str(),
            options.x, options.y,
            options.width, height,
            System::WindowOptions(window_options)));
    
        // Needs to be done after the window is created.
        audio::Initialize();

        mono::EventHandler event_handler;
        mono::SystemContext system_context;
        mono::Camera camera;

        mono::RenderInitParams render_params;
        render_params.pixels_per_meter = 32.0f;
        render_params.light_mask_texture = game_config.light_mask_texture.c_str();
        render_params.sprite_shadow_texture = game_config.sprite_shadow_texture.c_str();
        render_params.window = window.get();

        game::CreateGameSystems(max_entities, system_context, event_handler, camera, render_params, game_config);

        mono::IEntityManager* entity_manager = system_context.GetSystem<mono::IEntityManager>();

        game::RegisterGameComponents(entity_manager);
        game::RegisterSharedComponents(entity_manager);
        game::LoadFonts();

        game::ZoneCreationContext zone_context;
        zone_context.num_entities = max_entities;
        zone_context.event_handler = &event_handler;
        zone_context.game_config = &game_config;
        zone_context.system_context = &system_context;
        zone_context.window = window.get();

        game::ZoneManager(&camera, zone_context).Run(options.start_zone);

        system_context.DestroySystems();
    }

    audio::Shutdown();
    network::Shutdown();
    System::Shutdown();

    return 0;
}
