
#pragma once

#include "MonoFwd.h"
#include "IGameMode.h"
#include "Input/InputInterfaces.h"

namespace game
{
    class StartMenuGameMode : public IGameMode, public mono::IControllerInput, public mono::IKeyboardInput
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
        void GameOver();
        void Aborted();

        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;
        mono::InputResult KeyUp(const event::KeyUpEvent& event) override;

        mono::InputSystem* m_input_system;
        mono::EventHandler* m_event_handler;
        class TriggerSystem* m_trigger_system;
        class UISystem* m_ui_system;
        mono::InputContext* m_input_context;

        uint32_t m_level_completed_trigger;
        uint32_t m_level_gameover_trigger;
        uint32_t m_level_aborted_trigger;
        int m_game_mode_result;
    };
}
