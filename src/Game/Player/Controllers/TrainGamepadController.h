
#pragma once

#include "MonoFwd.h"
#include "Input/InputInterfaces.h"
#include "IUpdatable.h"
#include "System/System.h"

namespace game
{
    class TrainGamepadController : public mono::IControllerInput
    {
    public:

        TrainGamepadController(class TrainLogic* train_logic);
        void Update(const mono::UpdateContext& update_context);

    private:

        mono::InputResult ButtonDown(const event::ControllerButtonDownEvent& event) override;
        mono::InputResult Axis(const event::ControllerAxisEvent& event) override;
        mono::InputResult UpdatedControllerState(const System::ControllerState& updated_state) override;

        game::TrainLogic* m_train_logic;
        bool m_reverse;

        System::ControllerState m_current_state;
        System::ControllerState m_last_state;
    };
}
