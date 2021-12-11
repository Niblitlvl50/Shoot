
#include "EndScreen.h"
#include "ZoneFlow.h"

#include "IUpdatable.h"
#include "System/System.h"

#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"

namespace
{
    class CheckControllerInput : public mono::IUpdatable
    {
    public:

        CheckControllerInput(mono::EventHandler* event_handler)
            : m_event_handler(event_handler)
            , m_last_state{}
            , m_delay_counter_s(0.0f)
        { }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            m_delay_counter_s += update_context.delta_s;
            if(m_delay_counter_s > 1.0f)
            {
                if(state.button_state != m_last_state.button_state)
                    m_event_handler->DispatchEvent(event::QuitEvent());
            }

            m_last_state = state;
        }

        mono::EventHandler* m_event_handler;
        System::ControllerState m_last_state;
        float m_delay_counter_s;
    };
}

using namespace game;

EndScreen::EndScreen(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/end_screen.components")
    , m_event_handler(context.event_handler)
{ }

void EndScreen::OnLoad(mono::ICamera* camera, mono::IRenderer* renderer)
{
    GameZone::OnLoad(camera, renderer);
    AddUpdatable(new CheckControllerInput(m_event_handler));
}

int EndScreen::OnUnload()
{
    GameZone::OnUnload();
    return TITLE_SCREEN;
}
