
#pragma once

#include "MonoFwd.h"
#include "IGameMode.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"

#include "StateMachine.h"
#include "System/System.h"

#include <memory>

namespace game
{
    class PacketDeliveryGameMode : public IGameMode
    {
    public:

        PacketDeliveryGameMode();
        ~PacketDeliveryGameMode();

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const math::Vector& player_spawn) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;

        void ToFadeIn();
        void FadeIn(const mono::UpdateContext& update_context);

        void ToGameLogic();
        void GameLogic(const mono::UpdateContext& update_context);

        void ToPlayerDead();
        void PlayerDead(const mono::UpdateContext& update_context);

        void ToFadeOut();
        void FadeOut(const mono::UpdateContext& update_context);

        void UpdateOnPlayerState(const mono::UpdateContext& update_context);

        enum class GameModeStates
        {
            FADE_IN,
            RUN_GAME,
            PLAYER_DEAD,
            FADE_OUT
        };

        using GameModeStateMachine = StateMachine<GameModeStates, const mono::UpdateContext&>;
        GameModeStateMachine m_states;

        mono::IRenderer* m_renderer;
        mono::EventHandler* m_event_handler;
        class TriggerSystem* m_trigger_system;

        std::unique_ptr<class PlayerDaemon> m_player_daemon;

        std::unique_ptr<class BigTextScreen> m_fade_screen;
        std::unique_ptr<class PlayerUIElement> m_player_ui;

        mono::EventToken<struct GameOverEvent> m_gameover_token;

        int m_next_zone;
        uint32_t m_level_completed_trigger;

        float m_fade_in_timer;
        float m_fade_out_timer;

        System::ControllerState m_last_state;
    };
}
