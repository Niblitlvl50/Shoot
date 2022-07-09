
#include "System/System.h"
#include "Camera/Camera.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Lights/LightSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "EventHandler/EventHandler.h"
#include "Engine.h"
#include "SystemContext.h"

#include "TransformSystem/TransformSystem.h"
#include "EntitySystem/EntitySystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Paths/PathSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Particle/ParticleSystem.h"

#include "Entity/Component.h"
#include "Entity/ComponentFunctions.h"
#include "Entity/LoadEntity.h"

#include "DamageSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "World/WorldBoundsSystem.h"

#include "Editor.h"
#include "EditorConfig.h"
#include "FontIds.h"

int main()
{
    constexpr size_t max_entities = 1000;

    System::InitializeContext init_context;
    init_context.application = "Risky Delivery Inc";
    init_context.organization = "Nib-Games";
    init_context.working_directory = ".";
    System::Initialize(init_context);

    {
        editor::Config config;
        editor::LoadConfig("res/editor_config.json", config);

        System::IWindow* window = System::MakeWindow(
            "editor", config.window_position.x, config.window_position.y, config.window_size.x, config.window_size.y, System::WindowOptions::NONE);

        mono::RenderInitParams render_params;
        render_params.pixels_per_meter = 32.0f;
        render_params.light_mask_texture = "res/textures/lightmasks/light_mask_3.png";
        render_params.sprite_shadow_texture = "res/textures/roundshadow.png";
        render_params.imgui_ini = "res/editor_imgui.ini";
        render_params.window = window;
        mono::InitializeRender(render_params);

        mono::EventHandler event_handler;
        mono::SystemContext system_context;

        mono::EntitySystem* entity_system =
            system_context.CreateSystem<mono::EntitySystem>(max_entities, &system_context, game::LoadEntityFile, ComponentNameFromHash);

        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::TextSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::PathSystem>(max_entities, transform_system);
        system_context.CreateSystem<mono::RoadSystem>(max_entities);
        system_context.CreateSystem<mono::LightSystem>(max_entities);
        system_context.CreateSystem<mono::ParticleSystem>(max_entities, 100, transform_system);

        mono::PhysicsSystemInitParams physics_init_params;
        physics_init_params.n_bodies = max_entities;

        mono::PhysicsSystem* physics_system = system_context.CreateSystem<mono::PhysicsSystem>(physics_init_params, transform_system);
        game::DamageSystem* damage_system = system_context.CreateSystem<game::DamageSystem>(max_entities, transform_system, sprite_system, entity_system);
        system_context.CreateSystem<game::TriggerSystem>(max_entities, damage_system, physics_system, entity_system);
        system_context.CreateSystem<game::WorldBoundsSystem>(transform_system);

        game::RegisterSharedComponents(entity_system);
        game::LoadFonts();

        mono::Camera camera;
        auto editor = std::make_unique<editor::Editor>(window, *entity_system, event_handler, system_context, config, max_entities);
        mono::Engine(window, &camera, &system_context, &event_handler).Run(editor.get());

        const System::Position& position = window->Position();
        const System::Size& size = window->Size();
        config.window_position = math::Vector(position.x, position.y);
        config.window_size = math::Vector(size.width, size.height);

        editor::SaveConfig("res/editor_config.json", config);

        system_context.DestroySystems();
    
        delete window;
    }

    mono::ShutdownRender();
    System::Shutdown();

    return 0;
}


