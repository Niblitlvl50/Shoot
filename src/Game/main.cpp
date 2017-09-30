
#include "System/System.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"

#include "Engine.h"
#include "Rendering/Text/TextFunctions.h"
#include "EventHandler/EventHandler.h"

#include "Camera.h"
#include "TestZone.h"
#include "Factories.h"
#include "Weapons/WeaponFactory.h"
#include "Enemies/EnemyFactory.h"
#include "FontIds.h"

int main(int argc, char* argv[])
{
    System::Initialize();
    //System::SetCursorVisibility(System::CursorVisibility::HIDDEN);

    mono::InitializeAudio();
    mono::InitializeRender();

    {
        mono::EventHandler event_handler;

        game::weapon_factory = new game::WeaponFactory(event_handler);
        game::enemy_factory = new game::EnemyFactory(event_handler);
        
        constexpr math::Vector window_size(1280, 800);
        System::IWindow* window = System::CreateWindow("Game", window_size.x, window_size.y, false);
        window->SetBackgroundColor(0.6, 0.6, 0.6);
        
        mono::LoadFont(game::FontId::SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(game::FontId::MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(game::FontId::LARGE,  "res/pixelette.ttf", 10.0f, 1.0f);
        
        auto camera = std::make_shared<game::Camera>(32, 20, window_size.x, window_size.y, event_handler);
        mono::Engine engine(window, camera, event_handler);
        engine.Run(std::make_shared<game::TestZone>(event_handler));

        delete window;
        delete game::weapon_factory;
        delete game::enemy_factory;
    }

    mono::ShutdownRender();
    mono::ShutdownAudio();
    System::Shutdown();

    return 0;
}

