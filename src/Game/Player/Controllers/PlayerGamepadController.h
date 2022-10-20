
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "IUpdatable.h"
#include "System/System.h"

namespace game
{
    class PlayerGamepadController
    {
    public:
        
        PlayerGamepadController(
            class PlayerLogic* player_logic, mono::EventHandler* event_handler, const System::ControllerState& controller);
        ~PlayerGamepadController();
        void Update(const mono::UpdateContext& update_context);
        uint32_t GetLastInputTimestamp() const;
        
    private:
        
        game::PlayerLogic* m_player_logic;
        mono::EventHandler* m_event_handler;
        uint32_t m_last_input_timestamp;

        const System::ControllerState& m_state;
        System::ControllerState m_last_state;

        bool m_pause;
        mono::EventToken<event::ControllerButtonDownEvent> m_controller_token;
    };
}
