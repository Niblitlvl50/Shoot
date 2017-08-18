
#pragma once

#include "MonoFwd.h"
#include "System/System.h"

namespace game
{
    class Shuttle;
    
    class ShuttleGamepadController
    {
    public:
        
        ShuttleGamepadController(game::Shuttle* shuttle, mono::EventHandler& event_handler, const System::ControllerState& controller);
        void Update(unsigned int delta);
        
    private:
        
        game::Shuttle* m_shuttle;
        mono::EventHandler& m_event_handler;

        const System::ControllerState& m_state;
        System::ControllerState m_last_state;

        int m_current_weapon_index = 0;
    };
}
