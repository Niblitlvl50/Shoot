
#include "System/System.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Text/TextFunctions.h"
#include "Camera/TraceCamera.h"
#include "EventHandler/EventHandler.h"
#include "Engine.h"

#include "Editor.h"
#include "EditorConfig.h"

int main(int argc, const char* argv[])
{
    // This is assumed to be the file argument
    const char* file_name = (argc < 2) ? nullptr : argv[1];

    System::Initialize();
    mono::InitializeRender();

    {
        editor::Config config;
        config.camera_position = math::ZeroVec;
        config.camera_viewport = math::Quad(0, 0, 600.0f, 400.0f);
        config.window_size = math::Vector(1200.0f, 800.0f);
        
        editor::LoadConfig("res/editor_config.json", config);
        
        System::IWindow* window = System::CreateWindow("Editor", config.window_size.x, config.window_size.y, false);
        window->SetBackgroundColor(0.7, 0.7, 0.7);
        
        mono::ICameraPtr camera = std::make_shared<mono::TraceCamera>(config.window_size.x, config.window_size.y);
        camera->SetPosition(config.camera_position);
        camera->SetViewport(config.camera_viewport);

        mono::LoadFont(0, "res/pixelette.ttf", 10.0f, 1.0f);
        
        mono::EventHandler eventHandler;
        mono::Engine engine(window, camera, eventHandler);
        engine.Run(std::make_shared<editor::Editor>(window, eventHandler, file_name));

        const System::Size& size = window->Size();

        config.camera_position = camera->GetPosition();
        config.camera_viewport = camera->GetViewport();
        config.window_size = math::Vector(size.width, size.height);
        editor::SaveConfig("res/editor_config.json", config);
    
        delete window;
    }

    mono::ShutdownRender();
    System::Shutdown();

    return 0;
}


