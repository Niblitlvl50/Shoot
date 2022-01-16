
#include "Camera/Camera.h"
#include "EventHandler/EventHandler.h"
#include "System/Audio.h"
#include "System/System.h"
#include "System/Network.h"

#include "SystemContext.h"
#include "EntitySystem/EntitySystem.h"
#include "Particle/ParticleSystem.h"
#include "Paths/PathSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "TransformSystem/TransformSystem.h"

#include "Player/PlayerInfo.h"
#include "Factories.h"
#include "FontIds.h"
#include "GameConfig.h"
#include "SpriteResources.h"
#include "Weapons/WeaponFactory.h"
#include "Zones/ZoneManager.h"

#include "DamageSystem.h"
#include "Entity/AnimationSystem.h"
#include "Entity/EntityLogicSystem.h"
#include "GameCamera/CameraSystem.h"
#include "InteractionSystem/InteractionSystem.h"
#include "Pickups/PickupSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "SpawnSystem/SpawnSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "DialogSystem/DialogSystem.h"

#include "Network/ServerManager.h"
#include "Network/ClientManager.h"
#include "Network/NetworkMessage.h"

#include "Entity/ComponentFunctions.h"
#include "Entity/GameComponentFuncs.h"
#include "Entity/EntityLogicFactory.h"
#include "Entity/LoadEntity.h"
#include "Entity/Component.h"
#include "BackgroundMusic.h"

#include <cassert>
#include <cstring>

namespace
{
    struct Options
    {
        int x = 100;
        int y = 100;
        int width = 1200;
        int height = 800;
        int start_zone = 1;
        const char* game_config = "res/game_config.json";
        const char* log_file = "game_log.log";
    };

    Options ParseCommandline(int argc, char* argv[])
    {
        Options options;

        for(int index = 0; index < argc; ++index)
        {
            const char* arg = argv[index];
            if(std::strcmp("--position", arg) == 0)
            {
                assert((index + 2) < argc);
                options.x = atoi(argv[++index]);
                options.y = atoi(argv[++index]);

            }
            else if(std::strcmp("--size", arg) == 0)
            {
                assert((index + 2) < argc);
                options.width = atoi(argv[++index]);
                options.height = atoi(argv[++index]);
            }
            else if(std::strcmp("--zone", arg) == 0)
            {
                assert((index + 1) < argc);
                options.start_zone = atoi(argv[++index]);
            }
            else if(std::strcmp("--config", arg) == 0)
            {
                assert((index + 1) < argc);
                options.game_config = argv[++index];
            }
            else if(std::strcmp("--log-file", arg) == 0)
            {
                assert((index + 1) < argc);
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
    game::LoadAllSprites("res/sprites/all_sprite_files.json");
    game::LoadAllTextures("res/textures/all_textures.json");
    game::LoadAllWorlds("res/worlds/all_worlds.json");

    network::Initialize(game_config.port_range_start, game_config.port_range_end);
    game::PrintNetworkMessageSize();

    audio::Initialize();
    game::InitializeBackgroundMusic();

    mono::PhysicsSystemInitParams physics_system_params;
    physics_system_params.n_bodies = max_entities;
    physics_system_params.n_circle_shapes = max_entities;
    physics_system_params.n_segment_shapes = max_entities;
    physics_system_params.n_polygon_shapes = max_entities;

    game::InitializePlayerInfo();

    {
        //const System::WindowOptions window_options = System::WindowOptions::DISABLE_VSYNC;
        const int window_options = System::WindowOptions::NONE;
        System::IWindow* window = System::MakeWindow("game", options.x, options.y, options.width, options.height, System::WindowOptions(window_options));

        mono::RenderInitParams render_params;
        render_params.pixels_per_meter = 32.0f;
        render_params.light_mask_texture = "res/textures/light_mask_3.png";
        render_params.sprite_shadow_texture = "res/textures/roundshadow.png";
        render_params.window = window;
        mono::InitializeRender(render_params);

        game::LoadFonts();

        mono::EventHandler event_handler;
        mono::SystemContext system_context;
        mono::Camera camera;

        mono::EntitySystem* entity_system =
            system_context.CreateSystem<mono::EntitySystem>(max_entities, &system_context, game::LoadEntityFile, ComponentNameFromHash);
        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100);

        mono::PhysicsSystem* physics_system = system_context.CreateSystem<mono::PhysicsSystem>(physics_system_params, transform_system);
        mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::TextSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::PathSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::RoadSystem>(max_entities);
        system_context.CreateSystem<mono::LightSystem>(max_entities);

        game::DamageSystem* damage_system =
            system_context.CreateSystem<game::DamageSystem>(max_entities, transform_system, sprite_system, entity_system, &event_handler);
        game::TriggerSystem* trigger_system =
            system_context.CreateSystem<game::TriggerSystem>(max_entities, damage_system, physics_system, entity_system);
        system_context.CreateSystem<game::EntityLogicSystem>(max_entities);
        system_context.CreateSystem<game::SpawnSystem>(max_entities, trigger_system, entity_system, transform_system);
        system_context.CreateSystem<game::PickupSystem>(max_entities, physics_system, entity_system);
        system_context.CreateSystem<game::AnimationSystem>(max_entities, trigger_system, transform_system, sprite_system);
        system_context.CreateSystem<game::CameraSystem>(max_entities, &camera, transform_system, &event_handler, trigger_system);
        system_context.CreateSystem<game::InteractionSystem>(max_entities, transform_system, trigger_system);
        system_context.CreateSystem<game::DialogSystem>(max_entities);

        system_context.CreateSystem<game::ServerManager>(&event_handler, &game_config);
        system_context.CreateSystem<game::ClientManager>(&event_handler, &game_config);

        game::RegisterGameComponents(entity_system);
        game::RegisterSharedComponents(entity_system);

        game::WeaponFactory weapon_factory(entity_system, &system_context);
        game::EntityLogicFactory logic_factory(&system_context, event_handler);

        game::g_weapon_factory = &weapon_factory;
        game::g_logic_factory = &logic_factory;

        game::ZoneCreationContext zone_context;
        zone_context.num_entities = max_entities;
        zone_context.event_handler = &event_handler;
        zone_context.game_config = &game_config;
        zone_context.system_context = &system_context;

        game::ZoneManager(window, &camera, zone_context, options.start_zone).Run();

        system_context.DestroySystems();

        delete window;
    }

    mono::ShutdownRender();
    game::ShutdownBackgroundMusic();
    audio::Shutdown();

    network::Shutdown();
    System::Shutdown();

    return 0;
}
