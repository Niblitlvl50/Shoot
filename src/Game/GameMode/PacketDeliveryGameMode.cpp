
#include "PacketDeliveryGameMode.h"
#include "Hud/BigTextScreen.h"
#include "Hud/PauseScreen.h"
#include "Hud/ShopScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Hud/LevelTimerUIElement.h"
#include "Player/PlayerAuxiliaryDrawer.h"
#include "Zones/ZoneFlow.h"
#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "Events/PlayerEvents.h"
#include "DamageSystem/DamageSystem.h"
#include "Player/PlayerDaemonSystem.h"
#include "GameCamera/CameraSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "RenderLayers.h"
#include "Rendering/IRenderer.h"
#include "UI/UISystem.h"
#include "Weapons/WeaponSystem.h"

#include "Physics/PhysicsSystem.h"
#include "Rendering/RenderSystem.h"
#include "Rendering/Sprite/SpriteSystem.h"
#include "Rendering/Sprite/Sprite.h"
#include "TransformSystem/TransformSystem.h"

#include "Camera/ICamera.h"
#include "EntitySystem/IEntityManager.h"
#include "Events/QuitEvent.h"
#include "Events/PauseEvent.h"
#include "Events/EventFuncFwd.h"
#include "EventHandler/EventHandler.h"
#include "SystemContext.h"
#include "System/Hash.h"
#include "Zone/IZone.h"
#include "Math/EasingFunctions.h"

#include "Player/CoopPowerupManager.h"

#include "Entity/Component.h"
#include "Entity/EntityLogicSystem.h"
#include "Input/InputSystem.h"

#include "WorldFile.h"
#include "Debug/IDebugDrawer.h"

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

PacketDeliveryGameMode::PacketDeliveryGameMode()
    : m_package_spawned(false)
    , m_next_zone(game::ZoneResult::ZR_ABORTED)
    , m_package_release_callback(0)
    , m_package_entity_id(mono::INVALID_ID)
{
    const GameModeStateMachine::StateTable state_table = {
        GameModeStateMachine::MakeState(GameModeStates::FADE_IN, &PacketDeliveryGameMode::ToFadeIn, &PacketDeliveryGameMode::FadeIn, this),
        GameModeStateMachine::MakeState(GameModeStates::RUN_GAME_MODE, &PacketDeliveryGameMode::ToRunGameMode, &PacketDeliveryGameMode::RunGameMode, this),
        GameModeStateMachine::MakeState(GameModeStates::PACKAGE_DESTROYED, &PacketDeliveryGameMode::ToPackageDestroyed, &PacketDeliveryGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::TIMEOUT, &PacketDeliveryGameMode::ToTimeout, &PacketDeliveryGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::LEVEL_ABORTED, &PacketDeliveryGameMode::ToLevelAborted, &PacketDeliveryGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::LEVEL_COMPLETED, &PacketDeliveryGameMode::ToLevelCompleted, &PacketDeliveryGameMode::LevelCompleted, this),
        GameModeStateMachine::MakeState(GameModeStates::PAUSED, &PacketDeliveryGameMode::ToPaused, &PacketDeliveryGameMode::Paused, &PacketDeliveryGameMode::ExitPaused, this),
        GameModeStateMachine::MakeState(GameModeStates::FADE_OUT, &PacketDeliveryGameMode::ToFadeOut, &PacketDeliveryGameMode::FadeOut, this),
    };
    m_states.SetStateTableAndState(state_table, GameModeStates::FADE_IN);
}

PacketDeliveryGameMode::~PacketDeliveryGameMode() = default;

void PacketDeliveryGameMode::Begin(
    mono::IZone* zone,
    mono::IRenderer* renderer,
    mono::SystemContext* system_context,
    mono::EventHandler* event_handler,
    const LevelMetadata& level_metadata)
{
    m_event_handler = event_handler;
    m_render_system = system_context->GetSystem<mono::RenderSystem>();
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_transform_system = system_context->GetSystem<mono::TransformSystem>();
    m_sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();
    m_camera_system = system_context->GetSystem<game::CameraSystem>();
    mono::InputSystem* input_system = system_context->GetSystem<mono::InputSystem>();

    DamageSystem* damage_system = system_context->GetSystem<game::DamageSystem>();
    WeaponSystem* weapon_system = system_context->GetSystem<game::WeaponSystem>();
    UISystem* ui_system = system_context->GetSystem<game::UISystem>();

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
            const mono::ICamera* camera = m_camera_system->GetActiveCamera();
            m_shop_screen->ShowAt(camera->GetTargetPosition());
        }
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(level_completed_hash, level_event_callback, mono::INVALID_ID);
    m_level_gameover_trigger = m_trigger_system->RegisterTriggerCallback(level_gameover_hash, level_event_callback, mono::INVALID_ID);
    m_level_aborted_trigger = m_trigger_system->RegisterTriggerCallback(level_aborted_hash, level_event_callback, mono::INVALID_ID);
    m_show_shop_screen_trigger = m_trigger_system->RegisterTriggerCallback(show_shop_screen_hash, level_event_callback, mono::INVALID_ID);

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

    m_coop_power_manager = std::make_unique<CoopPowerupManager>(damage_system);

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

    m_shop_screen = std::make_unique<ShopScreen>(m_transform_system, m_entity_manager, event_handler, ui_system);
    m_shop_screen->Hide();

    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players, game::n_players, weapon_system, m_sprite_system);

    m_level_timer = level_metadata.time_limit_s;
    m_level_has_timelimit = (m_level_timer > 0);

    m_timer_screen = std::make_unique<LevelTimerUIElement>();
    m_timer_screen->SetSeconds(m_level_timer);
    if(!m_level_has_timelimit)
        m_timer_screen->Hide();

    zone->AddUpdatable(m_coop_power_manager.get());
    zone->AddUpdatableDrawable(m_big_text_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_pause_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_shop_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_timer_screen.get(), LayerId::UI);

    // Package
    m_package_aux_drawer = std::make_unique<PackageAuxiliaryDrawer>(m_transform_system);
    zone->AddDrawable(m_package_aux_drawer.get(), LayerId::GAMEOBJECTS_UI);
}

int PacketDeliveryGameMode::End(mono::IZone* zone)
{
    zone->RemoveUpdatable(m_coop_power_manager.get());
    zone->RemoveDrawable(m_package_aux_drawer.get());
    zone->RemoveUpdatableDrawable(m_big_text_screen.get());
    zone->RemoveUpdatableDrawable(m_pause_screen.get());
    zone->RemoveUpdatableDrawable(m_shop_screen.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());
    zone->RemoveUpdatableDrawable(m_timer_screen.get());

    if(m_package_entity_id != mono::INVALID_ID)
        m_entity_manager->RemoveReleaseCallback(m_package_entity_id, m_package_release_callback);

    m_event_handler->RemoveListener(m_gameover_token);
    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_gameover_hash, m_level_gameover_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(level_aborted_hash, m_level_aborted_trigger, mono::INVALID_ID);
    m_trigger_system->RemoveTriggerCallback(show_shop_screen_hash, m_show_shop_screen_trigger, mono::INVALID_ID);

    return m_next_zone;
}

void PacketDeliveryGameMode::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);

/*
    const char* state_text = nullptr;

    switch(m_states.ActiveState())
    {
    case GameModeStates::FADE_IN:
        state_text = "Fade In";
        break;
    case GameModeStates::RUN_GAME_MODE:
        state_text = "Run Game Mode";
        break;
    case GameModeStates::PACKAGE_DESTROYED:
        state_text = "Package Destroyed";
        break;
    case GameModeStates::TIMEOUT:
        state_text = "Time out";
        break;
    case GameModeStates::LEVEL_COMPLETED:
        state_text = "Level Completed";
        break;
    case GameModeStates::LEVEL_ABORTED:
        state_text = "Level Aborted";
        break;
    case GameModeStates::PAUSED:
        state_text = "Paused";
        break;
    case GameModeStates::FADE_OUT:
        state_text = "Fade Out";
        break;
    }

    //char state_text[512] = { 0 };
    g_debug_drawer->DrawScreenText(state_text, math::Vector(1.0f, 5.0f), mono::Color::OFF_WHITE);
*/
}

void PacketDeliveryGameMode::OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position)
{
    const uint32_t portal_entity_id = m_entity_manager->CreateEntity("res/entities/portal_green.entity").id;
    m_transform_system->SetTransform(portal_entity_id, math::CreateMatrixWithPosition(position));
    m_transform_system->SetTransformState(portal_entity_id, mono::TransformState::CLIENT);

    m_sprite_system->SetSpriteEnabled(player_entity_id, false);

    const mono::SpriteAnimationCallback on_begin_finished = [this, player_entity_id](uint32_t sprite_id) {
        m_sprite_system->SetSpriteEnabled(player_entity_id, true);
        mono::IBody* player_body = m_physics_system->GetBody(player_entity_id);
        player_body->ApplyLocalImpulse(math::Vector(80.0f, 0.0f), math::ZeroVec);
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

void PacketDeliveryGameMode::SpawnPackage(const math::Vector& position)
{
    if(!m_spawn_package)
        return;

    if(m_package_spawned)
        return;

    m_package_spawned = true;

    m_package_entity_id = m_player_system->SpawnPackageAt(position);

    mono::IBody* package_body = m_physics_system->GetBody(m_package_entity_id);
    package_body->ApplyLocalImpulse(math::Vector(80.0f, 0.0f), math::ZeroVec);

    const mono::ReleaseCallback release_callback = [this](uint32_t entity_id) {
        m_states.TransitionTo(GameModeStates::PACKAGE_DESTROYED);
        m_big_text_screen->SetSubText("Your package was destroyed.");
    };
    m_package_release_callback = m_entity_manager->AddReleaseCallback(m_package_entity_id, release_callback);

    System::Log("PacketDeliveryGameMode|Spawning package[id:%u] at position %.2f %.2f", m_package_entity_id, position.x, position.y);
}

void PacketDeliveryGameMode::ToFadeIn()
{
    const std::vector<BigTextScreen::FadePattern> fade_pattern = {
        { BigTextScreen::FadeState::FADE_IN,    tweak_values::fade_duration_s },
        { BigTextScreen::FadeState::SHOWN,      tweak_values::level_result_duration_s },
        { BigTextScreen::FadeState::FADE_OUT,   tweak_values::fade_duration_s },
    };

    const auto callback = [this]() {
        m_states.TransitionTo(GameModeStates::RUN_GAME_MODE);
        m_big_text_screen->Hide();
    };
    m_big_text_screen->ShowWithFadePattern(fade_pattern, callback);

    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_IN, 1.0f, 0.0f);
}
void PacketDeliveryGameMode::FadeIn(const mono::UpdateContext& update_context)
{ }

void PacketDeliveryGameMode::ToRunGameMode()
{ }
void PacketDeliveryGameMode::RunGameMode(const mono::UpdateContext& update_context)
{
    if(m_package_spawned)
    {
        m_level_timer -= update_context.delta_s;
        m_timer_screen->SetSeconds(m_level_timer);

        if(m_level_has_timelimit && m_level_timer < 1.0f)
            m_states.TransitionTo(GameModeStates::TIMEOUT);
    }
}

void PacketDeliveryGameMode::ToPackageDestroyed()
{
    TriggerLevelCompletedFade("Delivery failed!", "You lost the package...", game::ZoneResult::ZR_GAME_OVER);
}

void PacketDeliveryGameMode::ToTimeout()
{
    TriggerLevelCompletedFade("Delivery failed!", "Man, you were too slow!", game::ZoneResult::ZR_GAME_OVER);
}

void PacketDeliveryGameMode::ToLevelCompleted()
{
    TriggerLevelCompletedFade("Delivery Completed!", "Good Job", game::ZoneResult::ZR_COMPLETED);
}

void PacketDeliveryGameMode::ToLevelAborted()
{
    TriggerLevelCompletedFade("Deliver Aborted", "See you!", game::ZoneResult::ZR_ABORTED);
}

void PacketDeliveryGameMode::TriggerLevelCompletedFade(const char* text, const char* sub_text, int exit_zone)
{
    m_next_zone = exit_zone;

    m_pause_screen->Hide();

    m_big_text_screen->SetText(text);
    m_big_text_screen->SetSubText(sub_text);
    m_big_text_screen->SetAlpha(0.0f);

    const std::vector<BigTextScreen::FadePattern> fade_pattern = {
        { BigTextScreen::FadeState::FADE_IN,    tweak_values::fade_duration_s },
        { BigTextScreen::FadeState::SHOWN,      tweak_values::level_result_duration_s },
    };
    const auto callback = [this]() {
        m_states.TransitionTo(GameModeStates::FADE_OUT);
    };
    m_big_text_screen->ShowWithFadePattern(fade_pattern, callback);
}

void PacketDeliveryGameMode::LevelCompleted(const mono::UpdateContext& update_context)
{ }

void PacketDeliveryGameMode::ToPaused()
{
    const mono::ICamera* camera = m_camera_system->GetActiveCamera();
    m_pause_screen->ShowAt(camera->GetTargetPosition());
}
void PacketDeliveryGameMode::Paused(const mono::UpdateContext& update_context)
{ }
void PacketDeliveryGameMode::ExitPaused()
{
    m_pause_screen->Hide();
}

void PacketDeliveryGameMode::ToFadeOut()
{
    const std::vector<BigTextScreen::FadePattern> fade_pattern = {
        { BigTextScreen::FadeState::FADE_OUT, tweak_values::fade_duration_s },
    };
    const auto callback = [this]() {
        m_event_handler->DispatchEvent(event::QuitEvent());
        m_big_text_screen->Hide();
    };
    m_big_text_screen->ShowWithFadePattern(fade_pattern, callback);

    m_render_system->TriggerScreenFade(mono::ScreenFadeState::FADE_OUT, tweak_values::fade_duration_s, 0.0f);
}
void PacketDeliveryGameMode::FadeOut(const mono::UpdateContext& update_context)
{ }
