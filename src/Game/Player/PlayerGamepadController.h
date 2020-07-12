
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "System/System.h"

namespace game
{
    class PlayerLogic;
    
    class PlayerGamepadController
    {
    public:
        
        PlayerGamepadController(
            game::PlayerLogic* shuttle_logic, mono::EventHandler& event_handler, const System::ControllerState& controller);
        void Update(const mono::UpdateContext& update_context);
        
    private:
        
        game::PlayerLogic* m_shuttle_logic;
        mono::EventHandler& m_event_handler;

        const System::ControllerState& m_state;
        System::ControllerState m_last_state;

        int m_current_weapon_index = 0;
    };
}
