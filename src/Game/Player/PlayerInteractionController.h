
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "EventHandler/EventToken.h"

namespace game
{
    class PlayerLogic;
    struct PickupEvent;

    class PlayerInteractionController
    {
    public:
    
        PlayerInteractionController(PlayerLogic* shuttle_logic, mono::EventHandler& event_handler);
        ~PlayerInteractionController();

        bool OnPickup(const game::PickupEvent& event);

    private:
        PlayerLogic* m_shuttle_logic;
        mono::EventHandler& m_event_handler;
        mono::ISoundPtr m_pickup_sound;
        mono::EventToken<game::PickupEvent> m_pickup_token;
    };
}
