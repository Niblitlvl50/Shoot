
#include "StartMenuGameMode.h"
#include "Zones/ZoneFlow.h"
#include "TriggerSystem/TriggerSystem.h"
#include "UI/UISystem.h"
#include "WorldFile.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Input/InputSystem.h"
#include "SystemContext.h"

#include "System/Hash.h"
#include "EntitySystem/Entity.h"

using namespace game;

void StartMenuGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_input_system = system_context->GetSystem<mono::InputSystem>();
    m_trigger_system = system_context->GetSystem<mono::TriggerSystem>();
    m_ui_system = system_context->GetSystem<game::UISystem>();
    m_event_handler = event_handler;

    m_level_completed_hash = hash::Hash(level_metadata.completed_trigger.c_str());
    m_level_completed_alt_hash = hash::Hash(level_metadata.completed_alt_trigger.c_str());
    m_level_aborted_hash = hash::Hash(level_metadata.aborted_trigger.c_str());
    m_level_failed_hash = hash::Hash(level_metadata.failed_trigger.c_str());

    const mono::TriggerCallback level_hash_callback = [this](uint32_t trigger_id) {
        if(trigger_id == m_level_completed_hash)
            Completed();
        else if(trigger_id == m_level_completed_alt_hash)
            CompletedAlt();
        else if(trigger_id == m_level_aborted_hash)
            Aborted();
        else if(trigger_id == m_level_failed_hash)
            GameOver();
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(m_level_completed_hash, level_hash_callback, mono::INVALID_ID);
    m_level_completed_alt_trigger = m_trigger_system->RegisterTriggerCallback(m_level_completed_alt_hash, level_hash_callback, mono::INVALID_ID);
    m_level_aborted_trigger = m_trigger_system->RegisterTriggerCallback(m_level_aborted_hash, level_hash_callback, mono::INVALID_ID);
    m_level_failed_trigger = m_trigger_system->RegisterTriggerCallback(m_level_failed_hash, level_hash_callback, mono::INVALID_ID);

    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeIfHandled, "StartMenuGameMode");
    m_input_context->enabled = true;
    m_input_context->controller_input = this;
    m_input_context->keyboard_input = this;

    m_ui_system->Enable();
}

int StartMenuGameMode::End(mono::IZone* zone)
{
    m_ui_system->Disable();

    m_trigger_system->RemoveTriggerCallback(m_level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_completed_alt_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_failed_hash, m_level_failed_trigger, mono::INVALID_ID);

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

void StartMenuGameMode::CompletedAlt()
{
    m_game_mode_result = game::ZoneResult::ZR_COMPLETED_ALT;
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
