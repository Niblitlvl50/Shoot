
#include "GameOverScreen.h"
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
        { }

        void Update(const mono::UpdateContext& update_context)
        {
            const System::ControllerState& state = System::GetController(System::ControllerId::Primary);

            if(state.button_state != m_last_state.button_state)
                m_event_handler->DispatchEvent(event::QuitEvent());

            m_last_state = state;
        }

        mono::EventHandler* m_event_handler;
        System::ControllerState m_last_state;
    };
}

using namespace game;

GameOverScreen::GameOverScreen(const ZoneCreationContext& context)
    : GameZone(context, "res/worlds/game_over.components")
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
    return ZoneFlow::TITLE_SCREEN;
}