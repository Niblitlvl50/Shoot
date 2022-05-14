
#include "GameOverScreen.h"
#include "ZoneFlow.h"

#include "IUpdatable.h"
#include "System/System.h"

#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"

namespace
{
    class CheckControllerInput : public mono::IUpdatable
    {
    public:

        CheckControllerInput(mono::EventHandler* event_handler)
            : m_event_handler(event_handler)
            , m_send_quit(false)
            , m_last_state{}
            , m_delay_counter_s(0.0f)
        {
            const event::KeyUpEventFunc on_key_up = [this](const event::KeyUpEvent& event) {
                m_send_quit = true;
                return mono::EventResult::HANDLED;
            };
            m_key_up_token = m_event_handler->AddListener(on_key_up);
        }

        ~CheckControllerInput()
        {
            m_event_handler->RemoveListener(m_key_up_token);
        }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            m_delay_counter_s += update_context.delta_s;
            if(m_delay_counter_s > 0.5f)
            {
                if(state.button_state != m_last_state.button_state || m_send_quit)
                    m_event_handler->DispatchEvent(event::QuitEvent());
            }

            m_last_state = state;
        }

        mono::EventHandler* m_event_handler;
        mono::EventToken<event::KeyUpEvent> m_key_up_token;
        bool m_send_quit;

        System::ControllerState m_last_state;
        float m_delay_counter_s;
    };
}

using namespace game;

GameOverScreen::GameOverScreen(const ZoneCreationContext& context)
    : GameZone(context)
    , m_event_handler(context.event_handler)
{ }

void GameOverScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);
    AddUpdatable(new CheckControllerInput(m_event_handler));
}

int GameOverScreen::OnUnload()
{
    GameZone::OnUnload();
    return game::ZoneResult::ZR_COMPLETED;
}
