
#include "Camera/Camera.h"
#include "EventHandler/EventHandler.h"
#include "System/Audio.h"
#include "System/System.h"
#include "System/Network.h"

#include "SystemContext.h"
#include "EntitySystem/EntitySystem.h"
#include "Input/InputSystem.h"
#include "Particle/ParticleSystem.h"
#include "Paths/PathSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Player/PlayerInfo.h"
#include "Player/PlayerDaemonSystem.h"
#include "Factories.h"
#include "FontIds.h"
#include "GameConfig.h"
#include "Resources.h"
#include "Weapons/WeaponSystem.h"
#include "Zones/ZoneManager.h"

#include "DamageSystem/DamageSystem.h"
#include "Entity/AnimationSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "GameCamera/CameraSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Pickups/PickupSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "DialogSystem/DialogSystem.h"
#include "Sound/SoundSystem.h"
#include "World/RegionSystem.h"
#include "World/WorldBoundsSystem.h"
#include "UI/UISystem.h"

#include "Network/ServerManager.h"
#include "Network/ClientManager.h"
#include "Network/NetworkMessage.h"

#include "Entity/ComponentFunctions.h"
#include "Entity/GameComponentFuncs.h"
#include "Entity/EntityLogicFactory.h"
#include "Entity/LoadEntity.h"
#include "Entity/Component.h"

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
    constexpr size_t max_entities = 1000;
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

    mono::PhysicsSystemInitParams physics_system_params;
    physics_system_params.n_bodies = max_entities;
    physics_system_params.n_circle_shapes = max_entities;
    physics_system_params.n_segment_shapes = max_entities;
    physics_system_params.n_polygon_shapes = max_entities;

    {
        const System::Size window_size = System::GetCurrentWindowSize();
        const float window_ratio = float(window_size.width) / float(window_size.height);
        const int height = float(options.width) / window_ratio;

        const int window_options = 
            0;
            //System::WindowOptions::FULLSCREEN;
            //System::WindowOptions::FULLSCREEN_DESKTOP;
            //System::WindowOptions::DISABLE_VSYNC;
        System::IWindow* window = System::MakeWindow(
            game_config.application.c_str(),
            options.x, options.y,
            options.width, height,
            System::WindowOptions(window_options));
    
        // Needs to be done after the window is created.
        audio::Initialize();

        mono::RenderInitParams render_params;
        render_params.pixels_per_meter = 32.0f;
        render_params.light_mask_texture = game_config.light_mask_texture.c_str();
        render_params.sprite_shadow_texture = game_config.sprite_shadow_texture.c_str();
        render_params.window = window;
        mono::InitializeRender(render_params);

        game::LoadFonts();

        mono::EventHandler event_handler;
        mono::SystemContext system_context;
        mono::Camera camera;

        mono::InputSystem* input_system = system_context.CreateSystem<mono::InputSystem>(&event_handler);
        mono::EntitySystem* entity_system =
            system_context.CreateSystem<mono::EntitySystem>(max_entities, &system_context, game::LoadEntityFile, ComponentNameFromHash);
        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100, transform_system);

        mono::PhysicsSystem* physics_system = system_context.CreateSystem<mono::PhysicsSystem>(physics_system_params, transform_system);
        mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::TextSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::PathSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::RoadSystem>(max_entities);
        system_context.CreateSystem<mono::LightSystem>(max_entities);

        game::DamageSystem* damage_system =
            system_context.CreateSystem<game::DamageSystem>(max_entities, transform_system, sprite_system, entity_system);
        game::TriggerSystem* trigger_system =
            system_context.CreateSystem<game::TriggerSystem>(max_entities, damage_system, physics_system, entity_system);
        system_context.CreateSystem<game::EntityLogicSystem>(max_entities);
        system_context.CreateSystem<game::SpawnSystem>(max_entities, trigger_system, entity_system, transform_system);
        system_context.CreateSystem<game::PickupSystem>(max_entities, physics_system, entity_system);
        system_context.CreateSystem<game::AnimationSystem>(max_entities, trigger_system, transform_system, sprite_system);
        game::CameraSystem* camera_system =
            system_context.CreateSystem<game::CameraSystem>(max_entities, &camera, transform_system, &event_handler, trigger_system);
        system_context.CreateSystem<game::InteractionSystem>(max_entities, transform_system, trigger_system);
        system_context.CreateSystem<game::DialogSystem>(max_entities);
        system_context.CreateSystem<game::SoundSystem>(max_entities, trigger_system);
        system_context.CreateSystem<game::RegionSystem>(physics_system);
        system_context.CreateSystem<game::WorldBoundsSystem>(transform_system);
        system_context.CreateSystem<game::WeaponSystem>(
            transform_system, sprite_system, physics_system, damage_system, camera_system, entity_system, &system_context);
        system_context.CreateSystem<game::UISystem>(input_system, transform_system, entity_system, trigger_system);

        game::ServerManager* server_manager = system_context.CreateSystem<game::ServerManager>(&event_handler, &game_config);
        system_context.CreateSystem<game::ClientManager>(&event_handler, &game_config);

        system_context.CreateSystem<game::PlayerDaemonSystem>(server_manager, entity_system, &system_context, &event_handler);

        game::RegisterGameComponents(entity_system);
        game::RegisterSharedComponents(entity_system);

        game::EntityLogicFactory logic_factory(&system_context, event_handler);
        game::g_logic_factory = &logic_factory;

        game::ZoneCreationContext zone_context;
        zone_context.num_entities = max_entities;
        zone_context.event_handler = &event_handler;
        zone_context.game_config = &game_config;
        zone_context.system_context = &system_context;
        zone_context.window = window;

        game::ZoneManager(&camera, zone_context).Run(options.start_zone);

        system_context.DestroySystems();

        delete window;
    }

    mono::ShutdownRender();
    audio::Shutdown();

    network::Shutdown();
    System::Shutdown();

    return 0;
}
