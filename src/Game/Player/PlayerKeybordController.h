
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "IUpdatable.h"

#include "Math/Vector.h"

namespace game
{
    class PlayerKeyboardController
    {
    public:
        
        PlayerKeyboardController(class PlayerLogic* player_logic, mono::EventHandler* event_handler);
        ~PlayerKeyboardController();
        void Update(const mono::UpdateContext& update_context);
        uint32_t GetLastInputTimestamp() const;

    private:
        
        mono::EventResult OnMouseDown(const event::MouseDownEvent& event);
        mono::EventResult OnMouseUp(const event::MouseUpEvent& event);
        mono::EventResult OnMouseMotion(const event::MouseMotionEvent& event);
        mono::EventResult OnKeyDown(const event::KeyDownEvent& event);
        mono::EventResult OnKeyUp(const event::KeyUpEvent& event);

        game::PlayerLogic* m_player_logic;
        mono::EventHandler* m_event_handler;
        uint32_t m_last_input_timestamp;

        bool m_pause;
        bool m_fire;
        bool m_trigger_reload;
        bool m_trigger_action;
        bool m_trigger_pickup_drop;
        bool m_trigger_previous_weapon;
        bool m_trigger_next_weapon;
        bool m_update_aiming;

        mono::EventToken<event::MouseDownEvent> m_mouse_down_token;
        mono::EventToken<event::MouseUpEvent> m_mouse_up_token;
        mono::EventToken<event::MouseMotionEvent> m_mouse_motion_token;
        mono::EventToken<event::KeyDownEvent> m_key_down_token;
        mono::EventToken<event::KeyUpEvent> m_key_up_token;

        math::Vector m_aim_position;
    };
}
