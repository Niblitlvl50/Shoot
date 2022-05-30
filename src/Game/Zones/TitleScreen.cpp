
#include "TitleScreen.h"
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

        CheckControllerInput(TitleScreen* title_screen)
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

                if(continue_pressed)
                    m_title_screen->Continue();
                else if(quit_pressed)
                    m_title_screen->Quit();
            }

            m_last_state = state;
        }

        TitleScreen* m_title_screen;
        System::ControllerState m_last_state;
        float m_delay_counter_s;
    };
}

TitleScreen::TitleScreen(const ZoneCreationContext& context)
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
        return mono::EventResult::PASS_ON;
    };
    m_key_token = m_event_handler->AddListener(key_callback);
}

TitleScreen::~TitleScreen()
{
    m_event_handler->RemoveListener(m_key_token);
}

void TitleScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);
    AddUpdatable(new CheckControllerInput(this));
}

int TitleScreen::OnUnload()
{
    GameZone::OnUnload();
    return m_exit_zone;
}

void TitleScreen::Continue()
{
    m_exit_zone = game::ZoneResult::ZR_COMPLETED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void TitleScreen::Quit()
{
    m_exit_zone = game::ZoneResult::ZR_ABORTED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}
