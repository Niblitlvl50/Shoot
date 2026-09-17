
#include "TrainGameMode.h"
#include "Zones/ZoneFlow.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Behaviour/PathFollowerSystem.h"
#include "GameCamera/CameraSystem.h"
#include "WorldFile.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Input/InputSystem.h"
#include "SystemContext.h"

#include "EntitySystem/IEntityManager.h"
#include "System/Hash.h"
#include "System/System.h"

using namespace game;

void TrainGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_input_system = system_context->GetSystem<mono::InputSystem>();
    m_trigger_system = system_context->GetSystem<mono::TriggerSystem>();
    m_path_follower_system = system_context->GetSystem<game::PathFollowerSystem>();
    m_event_handler = event_handler;

    m_throttle_forward = false;
    m_throttle_backward = false;

    mono::IEntityManager* entity_manager = system_context->GetSystem<mono::IEntityManager>();
    const std::vector<uint32_t> train_entities = entity_manager->CollectEntitiesWithTag(hash::Hash("train"));
    m_train_entity_id = train_entities.empty() ? mono::INVALID_ID : train_entities.front();

    if(m_train_entity_id != mono::INVALID_ID)
    {
        game::CameraSystem* camera_system = system_context->GetSystem<game::CameraSystem>();
        camera_system->FollowEntity(m_train_entity_id);
    }
    else
    {
        System::Log("TrainGameMode|No entity tagged 'train' found, unable to drive.");
    }

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

    m_input_context = m_input_system->CreateContext(0, mono::InputContextBehaviour::ConsumeIfHandled, "TrainGameMode");
    m_input_context->enabled = true;
    m_input_context->keyboard_input = this;
}

int TrainGameMode::End(mono::IZone* zone)
{
    m_trigger_system->RemoveTriggerCallback(m_level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_completed_alt_hash, m_level_completed_alt_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_failed_hash, m_level_failed_trigger, mono::INVALID_ID);

    m_input_system->ReleaseContext(m_input_context);
    return m_game_mode_result;
}

void TrainGameMode::Update(const mono::UpdateContext& update_context)
{
    if(m_train_entity_id == mono::INVALID_ID)
        return;

    float throttle = 0.0f;
    if(m_throttle_forward)
        throttle += 1.0f;
    if(m_throttle_backward)
        throttle -= 1.0f;

    m_path_follower_system->SetThrottle(m_train_entity_id, throttle);
}

void TrainGameMode::Completed()
{
    m_game_mode_result = game::ZoneResult::ZR_COMPLETED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void TrainGameMode::CompletedAlt()
{
    m_game_mode_result = game::ZoneResult::ZR_COMPLETED_ALT;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void TrainGameMode::GameOver()
{
    m_game_mode_result = game::ZoneResult::ZR_GAME_OVER;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

void TrainGameMode::Aborted()
{
    m_game_mode_result = game::ZoneResult::ZR_ABORTED;
    m_event_handler->DispatchEvent(event::QuitEvent());
}

mono::InputResult TrainGameMode::KeyDown(const event::KeyDownEvent& event)
{
    switch(event.key)
    {
    case Keycode::W:
    case Keycode::UP:
        m_throttle_forward = true;
        break;
    case Keycode::S:
    case Keycode::DOWN:
        m_throttle_backward = true;
        break;
    default:
        break;
    }

    return mono::InputResult::Handled;
}

mono::InputResult TrainGameMode::KeyUp(const event::KeyUpEvent& event)
{
    switch(event.key)
    {
    case Keycode::W:
    case Keycode::UP:
        m_throttle_forward = false;
        break;
    case Keycode::S:
    case Keycode::DOWN:
        m_throttle_backward = false;
        break;
    default:
        break;
    }

    return mono::InputResult::Handled;
}
