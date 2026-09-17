
#pragma once

#include "MonoFwd.h"
#include "IGameMode.h"
#include "Input/InputInterfaces.h"

#include <cstdint>

namespace game
{
    // A minimal mode where the player drives a track-locked train (a "path_follower"
    // component with manual_control enabled) by throttling forward/backward. No combat.
    class TrainGameMode : public IGameMode //, public mono::IKeyboardInput
    {
    public:

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const struct LevelMetadata& level_metadata) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;

        void Completed();
        void CompletedAlt();
        void GameOver();
        void Aborted();

        //mono::InputResult KeyDown(const event::KeyDownEvent& event) override;
        //mono::InputResult KeyUp(const event::KeyUpEvent& event) override;

    private:

        mono::InputSystem* m_input_system;
        mono::EventHandler* m_event_handler;
        mono::TriggerSystem* m_trigger_system;
        class PathFollowerSystem* m_path_follower_system;
        class PlayerDaemonSystem* m_player_system;
        mono::InputContext* m_input_context;

        uint32_t m_train_entity_id;
        //bool m_throttle_forward;
        //bool m_throttle_backward;

        uint32_t m_level_completed_hash;
        uint32_t m_level_completed_alt_hash;
        uint32_t m_level_aborted_hash;
        uint32_t m_level_failed_hash;

        uint32_t m_level_completed_trigger;
        uint32_t m_level_completed_alt_trigger;
        uint32_t m_level_aborted_trigger;
        uint32_t m_level_failed_trigger;

        int m_game_mode_result;
    };
}
