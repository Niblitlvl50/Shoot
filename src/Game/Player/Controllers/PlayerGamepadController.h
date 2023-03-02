
#pragma once

#include "MonoFwd.h"
#include "Input/InputInterfaces.h"
#include "IUpdatable.h"
#include "System/System.h"

namespace game
{
    class PlayerGamepadController : public mono::IControllerInput
    {
    public:
        
        PlayerGamepadController(class PlayerLogic* player_logic);
        void Update(const mono::UpdateContext& update_context);

    private:

        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;
        mono::InputResult Axis(const event::ControllerAxisEvent& event) override;
        mono::InputResult UpdatedControllerState(const System::ControllerState& updated_state) override;

        game::PlayerLogic* m_player_logic;

        System::ControllerState m_current_state;
        System::ControllerState m_last_state;
    };
}
