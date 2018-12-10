
#include "System/System.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"

#include "Rendering/Text/TextFunctions.h"
#include "EventHandler/EventHandler.h"

#include "Camera.h"
#include "Factories.h"
#include "DamageController.h"
#include "Weapons/WeaponFactory.h"
#include "Enemies/EnemyFactory.h"
#include "GameObjects/GameObjectFactory.h"
#include "FontIds.h"
#include "Zones/ZoneManager.h"
#include "Physics/CMFactory.h"

int main(int argc, char* argv[])
{
    System::Initialize();

    mono::InitializeAudio();
    mono::InitializeRender(32.0f);

    mono::PhysicsInitParams init_params;
    init_params.n_bodies = 300;
    init_params.n_circle_shapes = 100;
    init_params.n_segment_shapes = 100;
    init_params.n_polygon_shapes = 200;

    mono::PhysicsFactory::Init(init_params);

    {
        mono::EventHandler event_handler;
        game::DamageController damage_controller;

        game::weapon_factory = new game::WeaponFactory(event_handler);
        game::enemy_factory = new game::EnemyFactory(event_handler, damage_controller);
        game::gameobject_factory = new game::GameObjectFactory(event_handler, damage_controller);

        constexpr math::Vector window_size(1280, 800);
        System::IWindow* window = System::CreateWindow("Game", 0, 0, window_size.x, window_size.y, false);
        window->SetBackgroundColor(0.0, 0.0, 0.0);
        
        mono::LoadFont(game::FontId::PIXELETTE_SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(game::FontId::PIXELETTE_MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(game::FontId::PIXELETTE_LARGE,  "res/pixelette.ttf", 10.0f, 1.0f / 3.0f);
        
        auto camera = std::make_shared<game::Camera>(22, 14, window_size.x, window_size.y, event_handler);

        game::ZoneCreationContext zone_context;
        zone_context.event_handler = &event_handler;
        zone_context.damage_controller = &damage_controller;

        game::ZoneManager zone_manager(window, camera, zone_context);
        zone_manager.Run();

        delete window;
        delete game::weapon_factory;
        delete game::enemy_factory;
        delete game::gameobject_factory;
    }

    mono::PhysicsFactory::Shutdown();
    mono::ShutdownRender();
    mono::ShutdownAudio();
    System::Shutdown();

    return 0;
}

