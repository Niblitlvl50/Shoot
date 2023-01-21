
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
#include "Input/InputSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Paths/PathSystem.h"
#include "RoadSystem/RoadSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Particle/ParticleSystem.h"
#include "UI/UISystem.h"
#include "GameCamera/CameraSystem.h"

#include "Entity/Component.h"
#include "Entity/ComponentFunctions.h"

#include "DamageSystem/DamageSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "World/RegionSystem.h"
#include "World/WorldBoundsSystem.h"
#include "Weapons/WeaponTypes.h"

#include "Editor.h"
#include "EditorConfig.h"
#include "FontIds.h"

int main()
{
    constexpr size_t max_entities = 1000;

    System::InitializeContext init_context;
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
        render_params.sprite_shadow_texture = "res/textures/shadows/roundshadow.png";
        render_params.imgui_ini = "res/editor_imgui.ini";
        render_params.window = window;

        mono::Camera camera;
        mono::EventHandler event_handler;
        mono::SystemContext system_context;

        mono::InputSystem* input_system = system_context.CreateSystem<mono::InputSystem>(&event_handler);
        system_context.CreateSystem<mono::RenderSystem>(max_entities, render_params);
        mono::EntitySystem* entity_system = system_context.CreateSystem<mono::EntitySystem>(
            max_entities, &system_context, component::ComponentNameFromHash, AttributeNameFromHash);

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
        game::TriggerSystem* trigger_system = system_context.CreateSystem<game::TriggerSystem>(max_entities, damage_system, physics_system, entity_system);
        game::CameraSystem* camera_system =
            system_context.CreateSystem<game::CameraSystem>(max_entities, &camera, transform_system, &event_handler, trigger_system);
        system_context.CreateSystem<game::RegionSystem>(physics_system);
        system_context.CreateSystem<game::WorldBoundsSystem>(transform_system);
        system_context.CreateSystem<game::UISystem>(input_system, transform_system, camera_system, trigger_system);

        game::RegisterSharedComponents(entity_system);
        game::LoadFonts();

        {
            auto editor = std::make_unique<editor::Editor>(window, *entity_system, event_handler, system_context, config, max_entities);
            mono::Engine(window, &camera, &system_context, &event_handler).Run(editor.get());

            const System::Position& position = window->Position();
            const System::Size& size = window->Size();
            config.window_position = math::Vector(position.x, position.y);
            config.window_size = math::Vector(size.width, size.height);

            editor::SaveConfig("res/editor_config.json", config);
        }

        system_context.DestroySystems();
    
        delete window;
    }

    System::Shutdown();

    return 0;
}


