
#include "TrainGameMode.h"
#include "Zones/ZoneFlow.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Behaviour/PathFollowerSystem.h"
#include "GameCamera/CameraSystem.h"
#include "Player/PlayerDaemonSystem.h"
#include "WorldFile.h"

#include "EventHandler/EventHandler.h"
#include "Events/QuitEvent.h"
#include "Input/InputSystem.h"
#include "Paths/PathSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/IRenderer.h"
#include "SystemContext.h"

#include "EntitySystem/IEntityManager.h"
#include "System/Hash.h"
#include "System/System.h"

namespace tweak_values
{
    constexpr float fade_duration_s = 2.0f;
}

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
    m_render_system = system_context->GetSystem<mono::RenderSystem>();
    m_event_handler = event_handler;

    renderer->SetScreenFadeAlpha(0.0f);
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_IN, tweak_values::fade_duration_s, 0.0f);

    // Player
    m_player_system = system_context->GetSystem<PlayerDaemonSystem>();
    m_player_system->SetTrainMode(true);

    const PlayerSpawnedCallback player_spawned_cb =
        [this, system_context](game::PlayerSpawnState spawn_state, uint32_t player_entity_id, const math::Vector& position) {

        float train_start_distance;
        const mono::PathSystem* path_system = system_context->GetSystem<mono::PathSystem>();
        const uint32_t path_entity_id = path_system->FindPathFromNotifierTag("player_start", train_start_distance);
        if(path_entity_id != mono::INVALID_ID)
        {
            const mono::IEntityManager* entity_manager = system_context->GetSystem<mono::IEntityManager>();
            const uint32_t path_uuid = entity_manager->GetEntityUuid(path_entity_id);
            m_path_follower_system->SetPathReference(player_entity_id, path_uuid, train_start_distance);
        }
    };
    m_player_system->SpawnPlayersAt(level_metadata.player_spawn_point, player_spawned_cb);

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
}

int TrainGameMode::End(mono::IZone* zone)
{
    m_trigger_system->RemoveTriggerCallback(m_level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_completed_alt_hash, m_level_completed_alt_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(m_level_failed_hash, m_level_failed_trigger, mono::INVALID_ID);

    return m_game_mode_result;
}

void TrainGameMode::Update(const mono::UpdateContext& update_context)
{
}

void TrainGameMode::Completed()
{
    TriggerFadeOutAndQuit(game::ZoneResult::ZR_COMPLETED);
}

void TrainGameMode::CompletedAlt()
{
    TriggerFadeOutAndQuit(game::ZoneResult::ZR_COMPLETED_ALT);
}

void TrainGameMode::GameOver()
{
    TriggerFadeOutAndQuit(game::ZoneResult::ZR_GAME_OVER);
}

void TrainGameMode::Aborted()
{
    TriggerFadeOutAndQuit(game::ZoneResult::ZR_ABORTED);
}

void TrainGameMode::TriggerFadeOutAndQuit(int zone_result)
{
    m_game_mode_result = zone_result;

    const mono::ScreenFadeCallback on_fade_complete = [this](mono::ScreenFadeState state) {
        m_event_handler->DispatchEvent(event::QuitEvent());
    };
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_OUT, tweak_values::fade_duration_s, 0.0f, on_fade_complete);
}
