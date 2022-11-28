
#include "StartMenuGameMode.h"
#include "Zones/ZoneFlow.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Input/InputSystem.h"
#include "SystemContext.h"

using namespace game;

void StartMenuGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_input_system = system_context->GetSystem<mono::InputSystem>();
    m_event_handler = event_handler;

    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeIfHandled);
    
    m_input_context->enabled = true;
    m_input_context->controller_input = this;
    m_input_context->keyboard_input = this;
}

int StartMenuGameMode::End(mono::IZone* zone)
{
    m_input_system->ReleaseContext(m_input_context);
    return m_game_mode_result;
}

void StartMenuGameMode::Update(const mono::UpdateContext& update_context)
{

}

mono::InputResult StartMenuGameMode::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    const bool top = event.button == System::ControllerButton::FACE_TOP;
    const bool bottom = event.button == System::ControllerButton::FACE_BOTTOM;
    if(top || bottom)
    {
        m_game_mode_result = top ? game::ZoneResult::ZR_ABORTED : game::ZoneResult::ZR_COMPLETED;
        m_event_handler->DispatchEvent(event::QuitEvent());
        return mono::InputResult::Handled;
    }

    return mono::InputResult::Pass;
}

mono::InputResult StartMenuGameMode::KeyUp(const event::KeyUpEvent& event)
{
    const bool enter = event.key == Keycode::ENTER;
    if(enter)
    {
        m_game_mode_result = game::ZoneResult::ZR_COMPLETED;
        m_event_handler->DispatchEvent(event::QuitEvent());
        return mono::InputResult::Handled;
    }

    return mono::InputResult::Pass;
}
