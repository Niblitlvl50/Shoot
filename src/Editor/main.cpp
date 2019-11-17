
#include "System/System.h"
#include "System/UID.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "Camera/TraceCamera.h"
#include "EventHandler/EventHandler.h"
#include "Engine.h"
#include "SystemContext.h"

#include "Editor.h"
#include "EditorConfig.h"
#include "FontIds.h"

#include "TransformSystem.h"
#include "EntitySystem.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "Component.h"
#include "Entity/EntityManager.h"
#include "Entity/ComponentFunctions.h"

int main(int argc, const char* argv[])
{
    // This is assumed to be the file argument
    const char* file_name = (argc < 2) ? nullptr : argv[1];

    constexpr size_t max_entities = 500;

    System::InitializeContext init_context;
    init_context.working_directory = ".";
    System::Initialize(init_context);
    System::SetUIDOffset(max_entities +1);

    mono::RenderInitParams render_params;
    render_params.pixels_per_meter = 32.0f;
    mono::InitializeRender(render_params);

    {
        mono::EventHandler event_handler;
        mono::SystemContext system_context;
        EntityManager entity_manager(&system_context);
        RegisterSharedComponents(entity_manager);

        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        system_context.CreateSystem<mono::EntitySystem>(max_entities);
        system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);

        editor::Config config;
        config.camera_position = math::ZeroVec;
        config.camera_viewport = math::Quad(0, 0, 600.0f, 400.0f);
        config.window_position = math::Vector(0.0f, 0.0f);
        config.window_size = math::Vector(1200.0f, 800.0f);
        config.draw_object_names = false;
        config.background_color = mono::Color::RGBA(0.7f, 0.7f, 0.7f, 1.0f);
        
        editor::LoadConfig("res/editor_config.json", config);
        
        System::IWindow* window = System::CreateWindow(
            "editor", config.window_position.x, config.window_position.y, config.window_size.x, config.window_size.y, false);
        
        mono::ICameraPtr camera = std::make_shared<mono::TraceCamera>(config.window_size.x, config.window_size.y);
        camera->SetPosition(config.camera_position);
        camera->SetViewport(config.camera_viewport);

        mono::LoadFont(editor::FontId::SMALL,       "res/pixelette.ttf", 10.0f, 1.0f / 15.0f);
        mono::LoadFont(editor::FontId::MEDIUM,      "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(editor::FontId::LARGE,       "res/pixelette.ttf", 10.0f, 1.0f /  5.0f);
        mono::LoadFont(editor::FontId::EXTRA_LARGE, "res/pixelette.ttf", 10.0f, 1.0f /  1.0f);

        auto editor = std::make_shared<editor::Editor>(window, entity_manager, event_handler, system_context, file_name);
        editor->EnableDrawObjectNames(config.draw_object_names);
        editor->EnableDrawSnappers(config.draw_snappers);
        editor->EnableDrawOutline(config.draw_outline);
        editor->SetBackgroundColor(config.background_color);

        mono::Engine(window, camera, &system_context, &event_handler).Run(editor.get());

        const System::Position& position = window->Position();
        const System::Size& size = window->Size();

        config.camera_position = camera->GetPosition();
        config.camera_viewport = camera->GetViewport();
        config.window_position = math::Vector(position.x, position.y);
        config.window_size = math::Vector(size.width, size.height);
        config.draw_object_names = editor->DrawObjectNames();
        config.draw_snappers = editor->DrawSnappers();
        config.draw_outline = editor->DrawOutline();
        config.background_color = editor->BackgroundColor();

        editor::SaveConfig("res/editor_config.json", config);
    
        delete window;
    }

    mono::ShutdownRender();
    System::Shutdown();

    return 0;
}


