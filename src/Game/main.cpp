
#include "System/System.h"
#include "Audio/AudioSystem.h"
#include "Rendering/RenderSystem.h"

#include "Engine.h"
#include "Camera/CameraSwitch.h"
#include "Camera/TraceCamera.h"
#include "Rendering/Text/TextFunctions.h"
#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"

#include "Camera.h"
#include "TestZone.h"
#include "Factories.h"
#include "Weapons/WeaponFactory.h"
#include "Enemies/EnemyFactory.h"
#include "FontIds.h"


namespace
{
    class CameraSwitchController
    {
    public:

        CameraSwitchController(std::shared_ptr<mono::CameraSwitch>& switcher, mono::EventHandler& event_handler)
            : m_switcher(switcher),
              m_event_handler(event_handler)
        {
            using namespace std::placeholders;
            const event::KeyUpEventFunc key_up_func = std::bind(&CameraSwitchController::OnKeyUp, this, _1);
            m_key_up_token = m_event_handler.AddListener(key_up_func);
        }

        ~CameraSwitchController()
        {
            m_event_handler.RemoveListener(m_key_up_token);
        }

        bool OnKeyUp(const event::KeyUpEvent& event)
        {
            if(event.key == Keycode::ONE)
                m_switcher->SetActiveCamera(0);
            else if(event.key == Keycode::TWO)
                m_switcher->SetActiveCamera(1);

            return true;
        }

        std::shared_ptr<mono::CameraSwitch> m_switcher;
        mono::EventHandler& m_event_handler;
        mono::EventToken<event::KeyUpEvent> m_key_up_token;
    };
}

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

        const std::vector<mono::ICameraPtr>& cameras = {
            std::make_shared<game::Camera>(32, 20),
            std::make_shared<game::DebugCamera>(32, 20, window_size.x, window_size.y, event_handler),
        };

        auto camera = std::make_shared<mono::CameraSwitch>(cameras);
        const CameraSwitchController camera_switch_controller(camera, event_handler);
        
        mono::LoadFont(game::FontId::SMALL,  "res/pixelette.ttf", 10.0f, 1.0f / 10.0f);
        mono::LoadFont(game::FontId::MEDIUM, "res/pixelette.ttf", 10.0f, 1.0f / 5.0f);
        mono::LoadFont(game::FontId::LARGE,  "res/pixelette.ttf", 10.0f, 1.0f);

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

