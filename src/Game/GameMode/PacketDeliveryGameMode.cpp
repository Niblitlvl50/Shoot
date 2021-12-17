
#include "PacketDeliveryGameMode.h"
#include "Hud/PlayerDeathScreen.h"
#include "Hud/PlayerUIElement.h"
#include "Zones/ZoneFlow.h"
#include "Events/GameEvents.h"
#include "Events/GameEventFuncFwd.h"
#include "Events/PlayerConnectedEvent.h"
#include "Player/PlayerDaemon.h"
#include "TriggerSystem/TriggerSystem.h"
#include "Network/ServerManager.h"
#include "RenderLayers.h"
#include "Rendering/IRenderer.h"


#include "EntitySystem/EntitySystem.h"
#include "Events/QuitEvent.h"
#include "EventHandler/EventHandler.h"
#include "SystemContext.h"
#include "System/Hash.h"
#include "Zone/IZone.h"
#include "Math/EasingFunctions.h"

namespace
{
    const uint32_t level_completed_hash = hash::Hash("level_completed");
}
namespace tweak_values
{
    constexpr float fade_duration_s = 1.0f;
}

using namespace game;

PacketDeliveryGameMode::PacketDeliveryGameMode()
    : m_next_zone(ZoneFlow::TITLE_SCREEN)
{
    const GameModeStateMachine::StateTable state_table = {
        GameModeStateMachine::MakeState(GameModeStates::FADE_IN, &PacketDeliveryGameMode::ToFadeIn, &PacketDeliveryGameMode::FadeIn, this),
        GameModeStateMachine::MakeState(GameModeStates::RUN_GAME_MODE, &PacketDeliveryGameMode::ToRunGameMode, &PacketDeliveryGameMode::RunGameMode, this),
        GameModeStateMachine::MakeState(GameModeStates::PLAYER_DEAD, &PacketDeliveryGameMode::ToPlayerDead, &PacketDeliveryGameMode::PlayerDead, this),
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
    const math::Vector& player_spawn)
{
    m_renderer = renderer;
    m_event_handler = event_handler;
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();

    // Quit and game over events
    const GameOverFunc on_game_over = [this](const game::GameOverEvent& game_over_event) {
        m_states.TransitionTo(GameModeStates::FADE_OUT);
        m_next_zone = game::ZoneFlow::GAME_OVER_SCREEN;
        return mono::EventResult::PASS_ON;
    };
    m_gameover_token = m_event_handler->AddListener(on_game_over);

    const TriggerCallback level_completed_callback = [this](uint32_t trigger_id) {
        m_states.TransitionTo(GameModeStates::FADE_OUT);
        m_next_zone = game::ZoneFlow::END_SCREEN;
    };
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(level_completed_hash, level_completed_callback, mono::INVALID_ID);

    // Player
    mono::EntitySystem* entity_system = system_context->GetSystem<mono::EntitySystem>();
    game::ServerManager* server_manager = system_context->GetSystem<game::ServerManager>();
    m_player_daemon = std::make_unique<PlayerDaemon>(server_manager, entity_system, system_context, m_event_handler, player_spawn);

    // UI
    m_dead_screen = std::make_unique<BigTextScreen>(
        "You are Dead!",
        "Press cross to try again, triangle to quit",
        mono::Color::RGBA(0.0f, 0.0f, 0.0f, 0.8f),
        mono::Color::BLACK,
        mono::Color::OFF_WHITE,
        mono::Color::GRAY);
    m_dead_screen->Hide();
    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players[0]);

    zone->AddUpdatableDrawable(m_dead_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
}

int PacketDeliveryGameMode::End(mono::IZone* zone)
{
    zone->RemoveUpdatableDrawable(m_dead_screen.get());
    zone->RemoveUpdatableDrawable(m_player_ui.get());

    m_event_handler->RemoveListener(m_gameover_token);
    m_trigger_system->RemoveTriggerCallback(level_completed_hash, m_level_completed_trigger, 0);

    return m_next_zone;
}

void PacketDeliveryGameMode::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void PacketDeliveryGameMode::ToFadeIn()
{
    m_fade_in_timer = 0.0f;
}
void PacketDeliveryGameMode::FadeIn(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseInCubic(m_fade_in_timer, tweak_values::fade_duration_s, 0.0f, 1.0f);
    m_renderer->SetScreenFadeAlpha(alpha);

    if(m_fade_in_timer > tweak_values::fade_duration_s)
        m_states.TransitionTo(GameModeStates::RUN_GAME_MODE);
    m_fade_in_timer += update_context.delta_s;
}

void PacketDeliveryGameMode::ToRunGameMode()
{ }
void PacketDeliveryGameMode::RunGameMode(const mono::UpdateContext& update_context)
{
    UpdateOnPlayerState(update_context);
}

void PacketDeliveryGameMode::ToPlayerDead()
{
    m_last_state.button_state = 0;
    m_dead_screen->Show();
}
void PacketDeliveryGameMode::PlayerDead(const mono::UpdateContext& update_context)
{
    PlayerInfo& player_info = game::g_players[0];

    const System::ControllerState& state = System::GetController(System::ControllerId::Primary);
    const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
    const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);

    m_last_state = state;

    GameModeStates new_game_mode;

    if(a_pressed)
    {
        m_event_handler->DispatchEvent(game::RespawnPlayerEvent(player_info.entity_id));
        new_game_mode = GameModeStates::RUN_GAME_MODE;
    }
    else if(y_pressed)
    {
        new_game_mode = GameModeStates::FADE_OUT;
    }

    if(a_pressed || y_pressed)
    {
        m_states.TransitionTo(new_game_mode);
        m_dead_screen->Hide();
    }
}

void PacketDeliveryGameMode::ToFadeOut()
{
    m_fade_out_timer = 0.0f;
    m_player_daemon->DespawnPlayer(&game::g_players[0]);
}
void PacketDeliveryGameMode::FadeOut(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseOutCubic(m_fade_out_timer, tweak_values::fade_duration_s, 1.0f, -1.0f);
    m_renderer->SetScreenFadeAlpha(alpha);

    if(m_fade_out_timer > tweak_values::fade_duration_s)
        m_event_handler->DispatchEvent(event::QuitEvent());
    m_fade_out_timer += update_context.delta_s;
}

void PacketDeliveryGameMode::UpdateOnPlayerState(const mono::UpdateContext& update_context)
{
    PlayerInfo& player_info = game::g_players[0];

    switch(player_info.player_state)
    {
    case game::PlayerState::NOT_SPAWNED:
        break;

    case game::PlayerState::ALIVE:
    {
        const bool is_in_dead_state = (m_states.ActiveState() == GameModeStates::PLAYER_DEAD);
        if(is_in_dead_state)
            m_states.TransitionTo(GameModeStates::FADE_IN);
        break;
    }

    case game::PlayerState::DEAD:
    {
        const bool is_in_game_state = (m_states.ActiveState() == GameModeStates::RUN_GAME_MODE);
        if(is_in_game_state)
            m_states.TransitionTo(GameModeStates::PLAYER_DEAD);
        break;
    }
    };
}
