
#pragma once

#include "MonoFwd.h"
#include "IGameMode.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"

#include "Events/EventFwd.h"
#include "StateMachine.h"

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
            const LevelMetadata& level_metadata) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;

        void OnSpawnPlayer(uint32_t player_entity_id, const math::Vector& position);
        void SpawnPackage(const math::Vector& position);

        void ToFadeIn();
        void FadeIn(const mono::UpdateContext& update_context);

        void ToRunGameMode();
        void RunGameMode(const mono::UpdateContext& update_context);

        void ToPackageDestroyed();
        void ToLevelCompleted();
        void ToLevelAborted();
        void TriggerLevelCompletedFade(const char* text, const char* sub_text, int exit_zone);
        void LevelCompleted(const mono::UpdateContext& update_context);

        void ToPaused();
        void Paused(const mono::UpdateContext& update_context);
        void ExitPaused();

        void ToFadeOut();
        void FadeOut(const mono::UpdateContext& update_context);

        enum class GameModeStates
        {
            FADE_IN,
            RUN_GAME_MODE,
            PACKAGE_DESTROYED,
            LEVEL_COMPLETED,
            LEVEL_ABORTED,
            PAUSED,
            FADE_OUT
        };

        using GameModeStateMachine = StateMachine<GameModeStates, const mono::UpdateContext&>;
        GameModeStateMachine m_states;

        mono::EventHandler* m_event_handler;
        mono::TriggerSystem* m_trigger_system;

        mono::RenderSystem* m_render_system;
        mono::IEntityManager* m_entity_manager;
        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::PhysicsSystem* m_physics_system;
        class PlayerDaemonSystem* m_player_system;
        class CameraSystem* m_camera_system;

        std::unique_ptr<class CoopPowerupManager> m_coop_power_manager;
        std::unique_ptr<class PackageAuxiliaryDrawer> m_package_aux_drawer;
        std::unique_ptr<class BigTextScreen> m_big_text_screen;
        std::unique_ptr<class PauseScreen> m_pause_screen;
        std::unique_ptr<class ShopScreen> m_shop_screen;
        std::unique_ptr<class PlayerUIElement> m_player_ui;

        mono::EventToken<struct GameOverEvent> m_gameover_token;
        mono::EventToken<event::PauseEvent> m_pause_token;

        bool m_spawn_package;
        bool m_package_spawned;
        math::Vector m_package_spawn_position;
        int m_next_zone;

        uint32_t m_level_completed_hash;
        uint32_t m_level_completed_alt_hash;
        uint32_t m_level_aborted_hash;
        uint32_t m_level_failed_hash;

        uint32_t m_level_completed_trigger;
        uint32_t m_level_aborted_trigger;
        uint32_t m_level_failed_trigger;
        uint32_t m_show_shop_screen_trigger;

        uint32_t m_package_release_callback;
        uint32_t m_package_entity_id;
    };
}
