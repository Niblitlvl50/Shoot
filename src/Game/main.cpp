
#include "System/System.h"
#include "System/File.h"
#include "System/Network.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "EventHandler/EventHandler.h"

#include "Factories.h"
#include "Weapons/WeaponFactory.h"
#include "FontIds.h"
#include "Zones/ZoneManager.h"
#include "GameConfig.h"
#include "AIKnowledge.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/EntitySystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Entity/EntityManager.h"
#include "Entity/EntityLogicSystem.h"
#include "DamageSystem.h"
#include "Particle/ParticleSystem.h"

#include "Entity/ComponentFunctions.h"
#include "Entity/GameComponentFuncs.h"
#include "Entity/EntityLogicFactory.h"

#include "SpriteResources.h"

#include <cassert>

namespace
{
    struct Options
    {
        int x = 0;
        int y = 0;
        int width = 1000;
        int height = 625;
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
            if(strcmp("--position", arg) == 0)
            {
                assert((index + 2) < argc);
                options.x = atoi(argv[++index]);
                options.y = atoi(argv[++index]);

            }
            else if(strcmp("--size", arg) == 0)
            {
                assert((index + 2) < argc);
                options.width = atoi(argv[++index]);
                options.height = atoi(argv[++index]);
            }
            else if(strcmp("--zone", arg) == 0)
            {
                assert((index + 1) < argc);
                options.start_zone = atoi(argv[++index]);
            }
            else if(strcmp("--config", arg) == 0)
            {
                assert((index + 1) < argc);
                options.game_config = argv[++index];
            }
            else if(strcmp("--log-file", arg) == 0)
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
    constexpr size_t max_entities = 500;
    const Options options = ParseCommandline(argc, argv);

    System::InitializeContext system_context;
    system_context.log_file = options.log_file;
    System::Initialize(system_context);

    game::Config game_config;
    game::LoadConfig(options.game_config, game_config);
    game::LoadAllSprites("res/sprites/all_sprite_files.json");

    network::Initialize(game_config.port_range_start, game_config.port_range_end);
    mono::InitializeAudio();

    mono::RenderInitParams render_params;
    render_params.pixels_per_meter = 32.0f;
    mono::InitializeRender(render_params);

    mono::PhysicsSystemInitParams physics_system_params;
    physics_system_params.n_bodies = max_entities;
    physics_system_params.n_circle_shapes = max_entities;
    physics_system_params.n_segment_shapes = max_entities;
    physics_system_params.n_polygon_shapes = max_entities;

    game::InitializeAIKnowledge();

    {
        mono::EventHandler event_handler;
        mono::SystemContext system_context;

        EntityManager entity_manager(&system_context);
        game::RegisterGameComponents(entity_manager);
        RegisterSharedComponents(entity_manager);

        game::WeaponFactory weapon_factory(&entity_manager, &system_context);
        game::EntityLogicFactory logic_factory(&system_context, event_handler);

        game::g_weapon_factory = &weapon_factory;
        game::g_logic_factory = &logic_factory;
        game::g_entity_manager = &entity_manager;

        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);

        system_context.CreateSystem<mono::EntitySystem>(max_entities);
        system_context.CreateSystem<game::EntityLogicSystem>(max_entities);
        system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::PhysicsSystem>(physics_system_params, transform_system);
        system_context.CreateSystem<game::DamageSystem>(max_entities, &entity_manager, &event_handler);
        system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100);

        const math::Vector window_size = math::Vector(options.width, options.height);
        System::IWindow* window = System::CreateWindow("game", options.x, options.y, window_size.x, window_size.y, false);
        window->SetBackgroundColor(0.7, 0.7, 0.7);
        
        mono::LoadFont(game::FontId::PIXELETTE_TINY,   "res/pixelette.ttf", 10.0f, 1.0f / 25.0f);
        mono::LoadFont(game::FontId::PIXELETTE_SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(game::FontId::PIXELETTE_MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(game::FontId::PIXELETTE_LARGE,  "res/pixelette.ttf", 10.0f, 1.0f / 3.0f);
        mono::LoadFont(game::FontId::PIXELETTE_MEGA,   "res/pixelette.ttf", 10.0f, 1.0f / 1.5f);

        game::ZoneCreationContext zone_context;
        zone_context.num_entities = max_entities;
        zone_context.event_handler = &event_handler;
        zone_context.game_config = &game_config;
        zone_context.system_context = &system_context;

        game::ZoneManager zone_manager(window, zone_context, options.start_zone);
        zone_manager.Run();

        delete window;
    }

    mono::ShutdownRender();
    mono::ShutdownAudio();

    network::Shutdown();
    System::Shutdown();

    return 0;
}
