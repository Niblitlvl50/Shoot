
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

        void OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position);
        void SpawnPackage(const math::Vector& position);

        void ToFadeIn();
        void FadeIn(const mono::UpdateContext& update_context);

        void ToRunGameMode();
        void RunGameMode(const mono::UpdateContext& update_context);

        void ToPackageDestroyed();
        void PackageDestroyed(const mono::UpdateContext& update_context);

        void ToFadeOut();
        void FadeOut(const mono::UpdateContext& update_context);

        enum class GameModeStates
        {
            FADE_IN,
            RUN_GAME_MODE,
            PACKAGE_DESTROYED,
            FADE_OUT
        };

        using GameModeStateMachine = StateMachine<GameModeStates, const mono::UpdateContext&>;
        GameModeStateMachine m_states;

        mono::IRenderer* m_renderer;
        mono::EventHandler* m_event_handler;
        class TriggerSystem* m_trigger_system;

        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::PhysicsSystem* m_physics_system;

        std::unique_ptr<class PlayerDaemon> m_player_daemon;
        std::unique_ptr<class BigTextScreen> m_dead_screen;
        std::unique_ptr<class PlayerUIElement> m_player_ui;
        mono::EventToken<struct GameOverEvent> m_gameover_token;

        bool m_package_spawned;
        int m_next_zone;

        uint32_t m_level_completed_trigger;
        uint32_t m_package_delivered_trigger;

        float m_fade_in_timer;
        float m_fade_out_timer;

        System::ControllerState m_last_state;
    };
}
