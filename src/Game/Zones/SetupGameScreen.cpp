
#include "SetupGameScreen.h"
#include "ZoneFlow.h"
#include "IUpdatable.h"

#include "Events/KeyEvent.h"
#include "Events/QuitEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"

#include "System/System.h"

using namespace game;

namespace
{
    class CheckControllerInput : public mono::IUpdatable
    {
    public:

        CheckControllerInput(SetupGameScreen* title_screen)
            : m_title_screen(title_screen)
            , m_last_state{}
            , m_delay_counter_s(0.0f)
        { }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            m_delay_counter_s += update_context.delta_s;
            if(m_delay_counter_s > 0.5f)
            {
                const bool continue_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_BOTTOM);
                const bool quit_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_TOP);
                const bool remote_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::FACE_LEFT);

                if(continue_pressed)
                    m_title_screen->Continue();
                else if(quit_pressed)
                    m_title_screen->Quit();
                else if(remote_pressed)
                    m_title_screen->Remote();
            }

            m_last_state = state;
        }

        SetupGameScreen* m_title_screen;
        System::ControllerState m_last_state;
        float m_delay_counter_s;
    };
}

SetupGameScreen::SetupGameScreen(const ZoneCreationContext& context)
    : GameZone(context)
    , m_event_handler(context.event_handler)
    , m_exit_zone(game::ZoneResult::ZR_ABORTED)
{
    const event::KeyUpEventFunc key_callback = [this](const event::KeyUpEvent& event)
    {
        if(event.key == Keycode::ENTER)
            Continue();
        else if(event.key == Keycode::Q)
            Quit();
        else if(event.key == Keycode::C)
            Remote();

        return mono::EventResult::PASS_ON;
    };
    m_key_token = m_event_handler->AddListener(key_callback);
}

SetupGameScreen::~SetupGameScreen()
{
    m_event_handler->RemoveListener(m_key_token);
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
    //m_exit_zone = TEST_ZONE;
    m_exit_zone = game::ZoneResult::ZR_COMPLETED;
    //m_exit_zone = ENEMY_TESTBED_ZONE;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void SetupGameScreen::Remote()
{
    //m_exit_zone = REMOTE_ZONE;
    //m_event_handler->DispatchEvent(event::QuitEvent());
}

void SetupGameScreen::Quit()
{
    m_exit_zone = game::ZoneResult::ZR_ABORTED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}
