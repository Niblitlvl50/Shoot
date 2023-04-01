
#include "HordeGameMode.h"

#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "GameCamera/CameraSystem.h"
#include "Hud/BigTextScreen.h"
#include "Hud/LevelTimerUIElement.h"
#include "Hud/PauseScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Player/PlayerDaemonSystem.h"
#include "Player/PlayerAuxiliaryDrawer.h"
#include "RenderLayers.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Weapons/WeaponSystem.h"
#include "WorldFile.h"
#include "Zones/ZoneFlow.h"


#include "Events/QuitEvent.h"
#include "Events/PauseEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"

#include "Camera/ICamera.h"
#include "Math/EasingFunctions.h"
#include "EntitySystem/IEntityManager.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"
#include "Zone/IZone.h"

#include "System/Hash.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");
    const uint32_t level_gameover_hash = hash::Hash("level_gameover");
    const uint32_t level_aborted_hash = hash::Hash("level_aborted");

    const uint32_t show_shop_screen_hash = hash::Hash("show_shop_screen");
}

namespace tweak_values
{
    constexpr float level_result_duration_s = 3.0f;
    constexpr float fade_duration_s = 1.0f;
}

using namespace game;

HordeGameMode::HordeGameMode()
{
    const GameModeStateMachine::StateTable state_table = {
        GameModeStateMachine::MakeState(GameModeStates::FADE_IN, &HordeGameMode::ToFadeIn, &HordeGameMode::FadeIn, this),
        GameModeStateMachine::MakeState(GameModeStates::RUN_GAME_MODE, &HordeGameMode::ToRunGameMode, &HordeGameMode::RunGameMode, this),
        GameModeStateMachine::MakeState(GameModeStates::PACKAGE_DESTROYED, &HordeGameMode::ToPackageDestroyed, &HordeGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::LEVEL_ABORTED, &HordeGameMode::ToLevelAborted, &HordeGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::LEVEL_COMPLETED, &HordeGameMode::ToLevelCompleted, &HordeGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::PAUSED, &HordeGameMode::ToPaused, &HordeGameMode::Paused, &HordeGameMode::ExitPaused, this),
        GameModeStateMachine::MakeState(GameModeStates::FADE_OUT, &HordeGameMode::ToFadeOut, &HordeGameMode::FadeOut, this),
    };
    m_states.SetStateTableAndState(state_table, GameModeStates::FADE_IN);
}

HordeGameMode::~HordeGameMode() = default;

void HordeGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_event_handler = event_handler;

    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_render_system = system_context->GetSystem<mono::RenderSystem>();
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_camera_system = system_context->GetSystem<game::CameraSystem>();

    mono::InputSystem* input_system = system_context->GetSystem<mono::InputSystem>();
    game::WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    game::UISystem* ui_system = system_context->GetSystem<game::UISystem>();

    SetupEvents();

    // Player
    m_player_system = system_context->GetSystem<PlayerDaemonSystem>();

    const PlayerSpawnedCallback player_spawned_cb =
        [this](game::PlayerSpawnState spawn_state, uint32_t player_entity_id, const math::Vector& position) {
        OnSpawnPlayer(player_entity_id, position);
    };
    m_player_system->SpawnPlayersAt(level_metadata.player_spawn_point, player_spawned_cb);

    m_spawn_package = level_metadata.spawn_package;
    m_package_spawn_position = level_metadata.use_package_spawn_position ?
        level_metadata.package_spawn_position : level_metadata.player_spawn_point;

    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players, game::n_players, weapon_system, m_sprite_system);

    m_level_timer = level_metadata.time_limit_s;
    m_timer_screen = std::make_unique<LevelTimerUIElement>();
    m_timer_screen->SetSeconds(m_level_timer);

    // UI
    m_big_text_screen = std::make_unique<BigTextScreen>(
        level_metadata.level_name.c_str(),
        level_metadata.level_description.c_str(),
        mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.0f),
        mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.0f),
        mono::Color::GOLDEN_YELLOW,
        mono::Color::GRAY,
        BigTextScreen::TEXT | BigTextScreen::SUBTEXT);
    m_big_text_screen->Hide();

    m_pause_screen = std::make_unique<PauseScreen>(m_transform_system, input_system, m_entity_manager, event_handler, ui_system);
    m_pause_screen->Hide();

    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_big_text_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_pause_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_timer_screen.get(), LayerId::UI);

    // Package
    m_package_aux_drawer = std::make_unique<PackageAuxiliaryDrawer>(m_transform_system);
    zone->AddDrawable(m_package_aux_drawer.get(), LayerId::GAMEOBJECTS_UI);
}

int HordeGameMode::End(mono::IZone* zone)
{
    zone->RemoveDrawable(m_package_aux_drawer.get());
    zone->RemoveUpdatableDrawable(m_big_text_screen.get());
    zone->RemoveUpdatableDrawable(m_pause_screen.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());
    zone->RemoveUpdatableDrawable(m_timer_screen.get());

    if(m_package_entity_id != mono::INVALID_ID)
        m_entity_manager->RemoveReleaseCallback(m_package_entity_id, m_package_release_callback);

    m_event_handler->RemoveListener(m_gameover_token);
    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_gameover_hash, m_level_gameover_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(show_shop_screen_hash, m_show_shop_screen_trigger, mono::INVALID_ID);

    return 0;
}

void HordeGameMode::Update(const mono::UpdateContext& update_context)
{
    m_level_timer += update_context.delta_s;
    m_timer_screen->SetSeconds(m_level_timer);

    m_states.UpdateState(update_context);
}

void HordeGameMode::SetupEvents()
{
    // Quit and game over events
    const GameOverFunc on_game_over = [this](const game::GameOverEvent& game_over_event) {
        m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        return mono::EventResult::PASS_ON;
    };
    m_gameover_token = m_event_handler->AddListener(on_game_over);

    const event::PauseEventFunc on_pause = [this](const event::PauseEvent& pause_event) {
        const GameModeStates previous_state = m_states.PreviousState();
        const GameModeStates new_state = pause_event.pause ? GameModeStates::PAUSED : previous_state;
        m_states.TransitionTo(new_state);

        return mono::EventResult::PASS_ON;
    };
    m_pause_token = m_event_handler->AddListener(on_pause);

    const TriggerCallback level_event_callback = [this](uint32_t trigger_id) {
        if(trigger_id == level_completed_hash)
            m_states.TransitionTo(GameModeStates::LEVEL_COMPLETED);
        else if(trigger_id == level_gameover_hash)
            m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        else if(trigger_id == level_aborted_hash)
            m_states.TransitionTo(GameModeStates::LEVEL_ABORTED);
        else if(trigger_id == show_shop_screen_hash)
        {
            //const mono::ICamera* camera = m_camera_system->GetActiveCamera();
            //m_shop_screen->ShowAt(camera->GetTargetPosition());
        }
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(level_completed_hash, level_event_callback, mono::INVALID_ID);
    m_level_gameover_trigger = m_trigger_system->RegisterTriggerCallback(level_gameover_hash, level_event_callback, mono::INVALID_ID);
    m_level_aborted_trigger = m_trigger_system->RegisterTriggerCallback(level_aborted_hash, level_event_callback, mono::INVALID_ID);
    m_show_shop_screen_trigger = m_trigger_system->RegisterTriggerCallback(show_shop_screen_hash, level_event_callback, mono::INVALID_ID);
}

void HordeGameMode::OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position)
{
    const uint32_t portal_entity_id = m_entity_manager->CreateEntity("res/entities/portal_green.entity").id;
    m_transform_system->SetTransform(portal_entity_id, math::CreateMatrixWithPosition(position));
    m_transform_system->SetTransformState(portal_entity_id, mono::TransformState::CLIENT);

    m_sprite_system->SetSpriteEnabled(player_entity_id, false);

    const mono::SpriteAnimationCallback on_begin_finished = [this, player_entity_id](uint32_t sprite_id) {
        m_sprite_system->SetSpriteEnabled(player_entity_id, true);
    };

    const mono::SpriteAnimationCallback on_idle_finished = [this](uint32_t sprite_id) {
        SpawnPackage(m_package_spawn_position);
    };

    const mono::SpriteAnimationCallback destroy_when_finish = [this, portal_entity_id](uint32_t sprite_id) {
        m_entity_manager->ReleaseEntity(portal_entity_id);
    };

    const std::vector<mono::SpriteAnimNode> anim_sequence = {
        { "begin", on_begin_finished },
        { "idle", on_idle_finished },
        { "end", destroy_when_finish }
    };
    m_sprite_system->RunSpriteAnimSequence(portal_entity_id, anim_sequence);
}

void HordeGameMode::SpawnPackage(const math::Vector& position)
{
    if(!m_spawn_package)
        return;

    if(m_package_spawned)
        return;

    m_package_spawned = true;

    m_package_entity_id = m_player_system->SpawnPackageAt(position);

    const mono::ReleaseCallback release_callback = [](uint32_t entity_id) {
        //m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        //m_big_text_screen->SetSubText("Your package was destroyed.");
    };
    m_package_release_callback = m_entity_manager->AddReleaseCallback(m_package_entity_id, release_callback);

    System::Log("HordeGameMode|Spawning package[id:%u] at position %.2f %.2f", m_package_entity_id, position.x, position.y);
}

void HordeGameMode::ToFadeIn()
{
    m_fade_timer = 0.0f; 
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_IN, 1.0f, 0.0f);
    m_big_text_screen->Show();
}

void HordeGameMode::FadeIn(const mono::UpdateContext& update_context)
{
    if(m_fade_timer < tweak_values::fade_duration_s)
    {
        const float alpha = math::EaseInCubic(m_fade_timer, tweak_values::fade_duration_s, 0.0f, 1.0f);
        m_big_text_screen->SetAlpha(alpha);
    }
    else if(m_fade_timer < tweak_values::fade_duration_s + 3.0f)
    {
        const float alpha = math::EaseInCubic(m_fade_timer - tweak_values::fade_duration_s, 3.0f, 1.0f, -1.0f);
        m_big_text_screen->SetAlpha(alpha);
    }
    else
    {
        m_big_text_screen->Hide();
        m_states.TransitionTo(GameModeStates::RUN_GAME_MODE);
    }

    m_fade_timer += update_context.delta_s;
}

void HordeGameMode::ToRunGameMode()
{}

void HordeGameMode::RunGameMode(const mono::UpdateContext& update_context)
{}

void HordeGameMode::ToPackageDestroyed()
{}

void HordeGameMode::ToTimeout()
{}

void HordeGameMode::ToLevelCompleted()
{}

void HordeGameMode::ToLevelAborted()
{
    m_next_zone = game::ZoneResult::ZR_ABORTED;

    m_pause_screen->Hide();

    m_big_text_screen->SetText("Horde Aborted!");
    m_big_text_screen->SetSubText("See you!");
    m_big_text_screen->SetAlpha(0.0f);
    m_big_text_screen->Show();

    m_fade_timer = 0.0f;
}

void HordeGameMode::LevelCompleted(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseInCubic(m_fade_timer, 1.0f, 0.0f, 1.0f);
    m_big_text_screen->SetAlpha(alpha);

    m_fade_timer += update_context.delta_s;
    if(m_fade_timer >= tweak_values::level_result_duration_s)
        m_states.TransitionTo(GameModeStates::FADE_OUT);
}

void HordeGameMode::ToPaused()
{
    const mono::ICamera* camera = m_camera_system->GetActiveCamera();
    m_pause_screen->ShowAt(camera->GetTargetPosition());
}
void HordeGameMode::Paused(const mono::UpdateContext& update_context)
{ }
void HordeGameMode::ExitPaused()
{
    m_pause_screen->Hide();
}

void HordeGameMode::ToFadeOut()
{
    m_fade_timer = 0.0f;
    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_OUT, tweak_values::fade_duration_s, 0.0f);
}
void HordeGameMode::FadeOut(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseOutCubic(m_fade_timer, tweak_values::fade_duration_s, 1.0f, -1.0f);
    m_big_text_screen->SetAlpha(alpha);

    if(m_fade_timer > tweak_values::fade_duration_s)
        m_event_handler->DispatchEvent(event::QuitEvent());
    m_fade_timer += update_context.delta_s;
}
