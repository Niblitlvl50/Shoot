
#include "System/System.h"
#include "System/File.h"

#include "SystemContext.h"
#include "Camera/Camera.h"
#include "EntitySystem/EntitySystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "TransformSystem/TransformSystem.h"

#include "EventHandler/EventHandler.h"
#include "Engine.h"

#include "Animator.h"

#include "Generated/pixelette.h"

#include <cstdio>

int main(int argc, const char* argv[])
{
    constexpr size_t max_entities = 500;

    System::InitializeContext init_context;
    init_context.working_directory = ".";
    init_context.log_file = "animator_log.log";
    System::Initialize(init_context);

    {
        System::IWindow* window = System::MakeWindow("Animator", 100, 100, 1200, 800, System::WindowOptions::VSYNC);

        mono::RenderInitParams render_params;
        render_params.window = window;

        mono::EventHandler event_handler;
        mono::SystemContext system_context;
    
        mono::RenderSystem* render_system = system_context.CreateSystem<mono::RenderSystem>(max_entities, render_params);
        mono::TransformSystem* transform_system = system_context.CreateSystem<mono::TransformSystem>(max_entities);
        mono::SpriteSystem* sprite_system = system_context.CreateSystem<mono::SpriteSystem>(max_entities, transform_system);
        mono::EntitySystem* entity_system = system_context.CreateSystem<mono::EntitySystem>(max_entities, &system_context, nullptr, nullptr);

        mono::LoadFontRaw(0, pixelette_data, pixelette_data_length, 48.0f, 0.01f);
        mono::LoadFontRaw(1, pixelette_data, pixelette_data_length, 48.0f, 0.05f);

        mono::Camera camera;
        mono::Engine engine(window, &camera, &system_context, &event_handler);

        animator::Animator animator(
            transform_system, sprite_system, render_system, entity_system, &event_handler, render_params.pixels_per_meter);
        engine.Run(&animator);

        delete window;
    }

    System::Shutdown();
    
    return 0;
}
