
#include "System/System.h"
#include "System/File.h"
#include "System/Network.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "Rendering/Text/TextSystem.h"
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
#include "Entity/ModificationSystem.h"
#include "DamageSystem.h"
#include "TriggerSystem.h"
#include "Particle/ParticleSystem.h"
#include "Pickups/PickupSystem.h"
#include "SpawnSystem.h"

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
        //int width = 1000;
        //int height = 625;
        int width = 530;
        int height = 900;
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
        //const System::WindowOptions window_options = System::WindowOptions::DISABLE_VSYNC;
        const System::WindowOptions window_options = System::WindowOptions::NONE;
        System::IWindow* window = System::CreateWindow("game", options.x, options.y, options.width, options.height, window_options);
        
        mono::LoadFont(shared::FontId::PIXELETTE_TINY,   "res/pixelette.ttf", 10.0f, 1.0f / 25.0f);
        mono::LoadFont(shared::FontId::PIXELETTE_SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(shared::FontId::PIXELETTE_MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(shared::FontId::PIXELETTE_LARGE,  "res/pixelette.ttf", 10.0f, 1.0f / 3.0f);
        mono::LoadFont(shared::FontId::PIXELETTE_MEGA,   "res/pixelette.ttf", 10.0f, 1.0f / 1.5f);

        mono::EventHandler event_handler;
        mono::SystemContext system_context;

        EntityManager entity_manager(&system_context);
        game::RegisterGameComponents(entity_manager);
        shared::RegisterSharedComponents(entity_manager);

        game::WeaponFactory weapon_factory(&entity_manager, &system_context);
        game::EntityLogicFactory logic_factory(&system_context, event_handler);

        game::g_weapon_factory = &weapon_factory;
        game::g_logic_factory = &logic_factory;
        game::g_entity_manager = &entity_manager;

        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        system_context.CreateSystem<mono::EntitySystem>(max_entities);

        mono::ParticleSystem* particle_system = system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100);
        mono::PhysicsSystem* physics_system = system_context.CreateSystem<mono::PhysicsSystem>(physics_system_params, transform_system);
        mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::TextSystem>(max_entities, transform_system);

        game::DamageSystem* damage_system = system_context.CreateSystem<game::DamageSystem>(
            max_entities, &entity_manager, particle_system, transform_system, physics_system, &event_handler);

        game::TriggerSystem* trigger_system = system_context.CreateSystem<game::TriggerSystem>(
            max_entities, damage_system, physics_system);

        system_context.CreateSystem<game::EntityLogicSystem>(max_entities);
        system_context.CreateSystem<game::SpawnSystem>(max_entities, transform_system);
        system_context.CreateSystem<game::PickupSystem>(max_entities, physics_system, &entity_manager);
        system_context.CreateSystem<game::ModificationSystem>(max_entities, trigger_system, transform_system, sprite_system);

        game::ZoneCreationContext zone_context;
        zone_context.num_entities = max_entities;
        zone_context.event_handler = &event_handler;
        zone_context.game_config = &game_config;
        zone_context.system_context = &system_context;

        game::ZoneManager zone_manager(window, zone_context, options.start_zone);
        zone_manager.Run();

        system_context.DestroySystems();

        delete window;
    }

    mono::ShutdownRender();
    mono::ShutdownAudio();

    network::Shutdown();
    System::Shutdown();

    return 0;
}
