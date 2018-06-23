
#include "System/System.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"

#include "Rendering/Text/TextFunctions.h"
#include "EventHandler/EventHandler.h"

#include "Camera.h"
#include "Factories.h"
#include "Weapons/WeaponFactory.h"
#include "Enemies/EnemyFactory.h"
#include "FontIds.h"
#include "Zones/ZoneManager.h"

int main(int argc, char* argv[])
{
    System::Initialize();

    mono::InitializeAudio();
    mono::InitializeRender();

    {
        mono::EventHandler event_handler;

        game::weapon_factory = new game::WeaponFactory(event_handler);
        game::enemy_factory = new game::EnemyFactory(event_handler);
        
        constexpr math::Vector window_size(1280, 800);
        System::IWindow* window = System::CreateWindow("Game", 0, 0, window_size.x, window_size.y, false);
        window->SetBackgroundColor(0.5, 0.5, 0.5);
        
        mono::LoadFont(game::FontId::PIXELETTE_SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(game::FontId::PIXELETTE_MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(game::FontId::PIXELETTE_LARGE,  "res/pixelette.ttf", 10.0f, 1.0f / 3.0f);
        
        auto camera = std::make_shared<game::Camera>(32, 20, window_size.x, window_size.y, event_handler);

        game::ZoneManager zone_manager(window, camera, event_handler);
        zone_manager.Run();

        delete window;
        delete game::weapon_factory;
        delete game::enemy_factory;
    }

    mono::ShutdownRender();
    mono::ShutdownAudio();
    System::Shutdown();

    return 0;
}

