
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "Input/InputSystem.h"
#include "Math/Vector.h"

namespace game
{
    class PlayerKeyboardController : public mono::IKeyboardInput, public mono::IMouseInput
    {
    public:
        
        PlayerKeyboardController(
            class PlayerLogic* player_logic, mono::InputSystem* input_system);
        ~PlayerKeyboardController();
        void Update(const mono::UpdateContext& update_context);
        uint32_t GetLastInputTimestamp() const;

    private:

        mono::InputResult KeyDown(const event::KeyDownEvent& event) override;
        mono::InputResult KeyUp(const event::KeyUpEvent& event) override;

        mono::InputResult Move(const event::MouseMotionEvent& event) override;
        mono::InputResult ButtonDown(const event::MouseDownEvent& event) override;
        mono::InputResult ButtonUp(const event::MouseUpEvent& event) override;

        game::PlayerLogic* m_player_logic;
        mono::InputSystem* m_input_system;

        mono::InputContext* m_input_context;

        uint32_t m_last_input_timestamp;

        bool m_fire;
        bool m_trigger_reload;
        bool m_trigger_action;
        bool m_trigger_pickup_drop;
        bool m_trigger_previous_weapon;
        bool m_trigger_next_weapon;
        bool m_update_aiming;
        bool m_trigger_respawn;
        math::Vector m_aim_position;
    };
}
