
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


        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;
        mono::InputResult KeyUp(const event::KeyUpEvent& event) override;

        mono::InputSystem* m_input_system;
        mono::EventHandler* m_event_handler;

        mono::InputContext* m_input_context;

        int m_game_mode_result;
    };
}
