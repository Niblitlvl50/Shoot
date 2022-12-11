
#pragma once

#include "MonoFwd.h"
#include "Input/InputSystem.h"
#include "IUpdatable.h"
#include "System/System.h"

namespace game
{
    class PlayerGamepadController : public mono::IControllerInput
    {
    public:
        
        PlayerGamepadController(
            class PlayerLogic* player_logic,
            mono::InputSystem* input_system,
            const System::ControllerState& controller);
        ~PlayerGamepadController();
        void Update(const mono::UpdateContext& update_context);
        uint32_t GetLastInputTimestamp() const;
        
    private:

        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;
        mono::InputResult Axis(const event::ControllerAxisEvent& event) override;

        game::PlayerLogic* m_player_logic;
        mono::InputSystem* m_input_system;
        uint32_t m_last_input_timestamp;

        mono::InputContext* m_input_context;

        const System::ControllerState& m_state;
        System::ControllerState m_last_state;
    };
}
