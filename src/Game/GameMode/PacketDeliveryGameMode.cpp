
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

    mono::EventResult GameOverAndQuit(int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::GAME_OVER_SCREEN;

        const auto send_quit = [](void* data) {
            mono::EventHandler* event_handler = static_cast<mono::EventHandler*>(data);
            event_handler->DispatchEvent(event::QuitEvent());
        };
        System::CreateTimer(1000, System::TimerProperties::AUTO_DELETE | System::TimerProperties::ONE_SHOT, send_quit, event_handler);

        return mono::EventResult::PASS_ON;
    }

    void GameCompleted(int32_t trigger_id, int& next_zone, mono::EventHandler* event_handler)
    {
        next_zone = game::ZoneFlow::END_SCREEN;
        event_handler->DispatchEvent(event::QuitEvent());
    }
}


using namespace game;

PacketDeliveryGameMode::PacketDeliveryGameMode()
    : m_next_zone(ZoneFlow::TITLE_SCREEN)
{
    const GameModeStateMachine::StateTable state_table = {
        GameModeStateMachine::MakeState(GameModeStates::FADE_IN, &PacketDeliveryGameMode::ToFadeIn, &PacketDeliveryGameMode::FadeIn, this),
        GameModeStateMachine::MakeState(GameModeStates::RUN_GAME, &PacketDeliveryGameMode::ToGameLogic, &PacketDeliveryGameMode::GameLogic, this),
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

    mono::EntitySystem* entity_system = system_context->GetSystem<mono::EntitySystem>();
    game::ServerManager* server_manager = system_context->GetSystem<game::ServerManager>();
    m_trigger_system = system_context->GetSystem<game::TriggerSystem>();

    const GameOverFunc on_game_over = std::bind(GameOverAndQuit, std::ref(m_next_zone), m_event_handler);
    m_gameover_token = m_event_handler->AddListener(on_game_over);

    // Player
    m_player_daemon = std::make_unique<PlayerDaemon>(
        server_manager, entity_system, system_context, m_event_handler, player_spawn);

    using namespace std::placeholders;
    m_level_completed_trigger = m_trigger_system->RegisterTriggerCallback(
        level_completed_hash, std::bind(GameCompleted, _1, std::ref(m_next_zone), m_event_handler), mono::INVALID_ID);

    m_fade_screen = std::make_unique<BigTextScreen>(
        "You are Dead!", "Press button to continue", mono::Color::BLACK, mono::Color::BLACK, mono::Color::OFF_WHITE, mono::Color::GRAY);
    m_player_ui = std::make_unique<PlayerUIElement>(game::g_players[0]);

    //zone->AddUpdatableDrawable(m_fade_screen.get(), LayerId::UI);
    zone->AddUpdatableDrawable(m_player_ui.get(), LayerId::UI);
}

int PacketDeliveryGameMode::End(mono::IZone* zone)
{
    //zone->RemoveUpdatableDrawable(m_fade_screen.get());
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
    const float alpha = math::EaseInCubic(m_fade_in_timer, 2.0f, 0.0f, 1.0f);
    m_renderer->SetScreenFadeAlpha(alpha);
    //m_fade_screen->SetAlpha(alpha);

    if(m_fade_in_timer > 2.0f)
        m_states.TransitionTo(GameModeStates::RUN_GAME);
    m_fade_in_timer += update_context.delta_s;
}

void PacketDeliveryGameMode::ToGameLogic()
{
}

void PacketDeliveryGameMode::GameLogic(const mono::UpdateContext& update_context)
{
    UpdateOnPlayerState(update_context);
}

void PacketDeliveryGameMode::ToPlayerDead()
{
    m_last_state.button_state = 0;
}

void PacketDeliveryGameMode::PlayerDead(const mono::UpdateContext& update_context)
{
    PlayerInfo& player_info = game::g_players[0];

    const System::ControllerState& state = System::GetController(System::ControllerId::Primary);
    const bool a_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::A);
    const bool y_pressed = System::ButtonTriggeredAndChanged(m_last_state.button_state, state.button_state, System::ControllerButton::Y);

    if(a_pressed)
        m_event_handler->DispatchEvent(game::RespawnPlayerEvent(player_info.entity_id));
    else if(y_pressed)
        m_event_handler->DispatchEvent(event::QuitEvent());

    m_last_state = state;
}

void PacketDeliveryGameMode::ToFadeOut()
{
    m_fade_out_timer = 0.0f;
}

void PacketDeliveryGameMode::FadeOut(const mono::UpdateContext& update_context)
{
    const float alpha = math::EaseOutCubic(m_fade_out_timer, 2.0f, 1.0f, -1.0f);
    //m_fade_screen->SetAlpha(alpha);
    m_renderer->SetScreenFadeAlpha(alpha);

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
        const bool is_in_game_state = (m_states.ActiveState() == GameModeStates::RUN_GAME);
        if(is_in_game_state)
            m_states.TransitionTo(GameModeStates::FADE_OUT);
        break;
    }
    };
}
