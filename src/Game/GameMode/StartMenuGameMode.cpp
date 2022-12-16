
#include "StartMenuGameMode.h"
#include "Zones/ZoneFlow.h"
#include "TriggerSystem/TriggerSystem.h"
#include "UI/UISystem.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Input/InputSystem.h"
#include "SystemContext.h"

#include "System/Hash.h"
#include "EntitySystem/Entity.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");
    const uint32_t level_gameover_hash = hash::Hash("level_gameover");
    const uint32_t level_aborted_hash = hash::Hash("level_aborted");
}

using namespace game;

void StartMenuGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_input_system = system_context->GetSystem<mono::InputSystem>();
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();
    m_ui_system = system_context->GetSystem<game::UISystem>();
    m_event_handler = event_handler;

    const TriggerCallback level_hash_callback = [this](uint32_t trigger_id) {
        if(trigger_id == level_completed_hash)
            Completed();
        else if(trigger_id == level_gameover_hash)
            GameOver();
        else if(trigger_id == level_aborted_hash)
            Aborted();
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(level_completed_hash, level_hash_callback, mono::INVALID_ID);
    m_level_gameover_trigger = m_trigger_system->RegisterTriggerCallback(level_gameover_hash, level_hash_callback, mono::INVALID_ID);
    m_level_aborted_trigger = m_trigger_system->RegisterTriggerCallback(level_aborted_hash, level_hash_callback, mono::INVALID_ID);

    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeIfHandled, "StartMenuGameMode");
    m_input_context->enabled = true;
    m_input_context->controller_input = this;
    m_input_context->keyboard_input = this;

    m_ui_system->Enable();
}

int StartMenuGameMode::End(mono::IZone* zone)
{
    m_ui_system->Disable();

    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_gameover_hash, m_level_gameover_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);

    m_input_system->ReleaseContext(m_input_context);
    return m_game_mode_result;
}

void StartMenuGameMode::Update(const mono::UpdateContext& update_context)
{

}

void StartMenuGameMode::Completed()
{
    m_game_mode_result = game::ZoneResult::ZR_COMPLETED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void StartMenuGameMode::GameOver()
{
    m_game_mode_result = game::ZoneResult::ZR_GAME_OVER;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void StartMenuGameMode::Aborted()
{
    m_game_mode_result = game::ZoneResult::ZR_ABORTED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

mono::InputResult StartMenuGameMode::ButtonDown(const event::ControllerButtonDownEvent& event)
{
    const bool top = event.button == System::ControllerButton::FACE_TOP;
    const bool bottom = event.button == System::ControllerButton::FACE_BOTTOM;
    if(top)
        Aborted();
    else if(bottom)
        Completed();

    return mono::InputResult::Handled;
}

mono::InputResult StartMenuGameMode::KeyUp(const event::KeyUpEvent& event)
{
    const bool enter = event.key == Keycode::ENTER && !event.alt;
    if(enter)
        Completed();

    return mono::InputResult::Handled;
}
