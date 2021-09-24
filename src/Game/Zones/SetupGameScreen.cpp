
#include "SetupGameScreen.h"
#include "ZoneFlow.h"
#include "Effects/ScreenSparkles.h"

#include "RenderLayers.h"

#include "Math/Quad.h"
#include "Camera/ICamera.h"
#include "Rendering/Color.h"

#include "Events/KeyEvent.h"
#include "Events/QuitEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"

#include "SystemContext.h"
#include "Particle/ParticleSystem.h"
#include "Particle/ParticleSystemDrawer.h"
#include "Rendering/Sprite/SpriteBatchDrawer.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Text/TextSystem.h"
#include "Rendering/Text/TextBatchDrawer.h"
#include "TransformSystem/TransformSystem.h"
#include "System/System.h"

#include "EntitySystem/IEntityManager.h"
#include "WorldFile.h"

using namespace game;

namespace
{
    class CheckControllerInput : public mono::IUpdatable
    {
    public:

        CheckControllerInput(SetupGameScreen* title_screen)
            : m_title_screen(title_screen)
            , m_last_state{}
        { }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
            const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);
            const bool x_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::X);

            if(a_pressed)
                m_title_screen->Continue();
            else if(y_pressed)
                m_title_screen->Quit();
            else if(x_pressed)
                m_title_screen->Remote();

            m_last_state = state;
        }

        SetupGameScreen* m_title_screen;
        System::ControllerState m_last_state;
    };
}

SetupGameScreen::SetupGameScreen(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/setup_game_screen.components")
    , m_event_handler(*context.event_handler)
    , m_system_context(context.system_context)
    , m_exit_zone(ZoneFlow::TITLE_SCREEN)
{
    const event::KeyUpEventFunc key_callback =
        [this](const event::KeyUpEvent& event) -> mono::EventResult
    {
        if(event.key == Keycode::ENTER)
            Continue();
        else if(event.key == Keycode::Q)
            Quit();
        else if(event.key == Keycode::C)
            Remote();

        return mono::EventResult::PASS_ON;
    };

    m_key_token = m_event_handler.AddListener(key_callback);
}

SetupGameScreen::~SetupGameScreen()
{
    m_event_handler.RemoveListener(m_key_token);
}

void SetupGameScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);
    AddUpdatable(new CheckControllerInput(this));
}

int SetupGameScreen::OnUnload()
{
    GameZone::OnUnload();
    return m_exit_zone;
}

void SetupGameScreen::Continue()
{
    m_exit_zone = TEST_ZONE;
    //m_exit_zone = TINY_ARENA_ZONE;
    m_event_handler.DispatchEvent(event::QuitEvent());
}

void SetupGameScreen::Remote()
{
    m_exit_zone = REMOTE_ZONE;
    m_event_handler.DispatchEvent(event::QuitEvent());
}

void SetupGameScreen::Quit()
{
    m_exit_zone = TITLE_SCREEN;
    m_event_handler.DispatchEvent(event::QuitEvent());
}
