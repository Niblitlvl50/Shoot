
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Input/InputInterfaces.h"

namespace game
{
    class TrainKeyboardController : public mono::IKeyboardInput
    {
    public:

        TrainKeyboardController(class TrainLogic* train_logic);
        void Update(const mono::UpdateContext& update_context);

    private:

        mono::InputResult KeyDown(const event::KeyDownEvent& event) override;
        mono::InputResult KeyUp(const event::KeyUpEvent& event) override;

        game::TrainLogic* m_train_logic;

        bool m_raise_throttle;
        bool m_lower_throttle;
        bool m_load_held;
        bool m_toggle_direction;
        bool m_honk;
        bool m_decouple;
        bool m_trigger_pickup_drop;
        bool m_trigger_respawn;
    };
}
