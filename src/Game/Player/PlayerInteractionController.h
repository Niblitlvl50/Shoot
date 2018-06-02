
#pragma once

#include "MonoFwd.h"
#include "EventHandler/EventToken.h"

namespace game
{
    class Shuttle;
    struct PickupEvent;

    class PlayerInteractionController
    {
    public:
    
        PlayerInteractionController(Shuttle* player, mono::EventHandler& event_handler);
        ~PlayerInteractionController();

        bool OnPickup(const game::PickupEvent& event);

    private:
        Shuttle* m_player;
        mono::EventHandler& m_event_handler;

        mono::EventToken<game::PickupEvent> m_pickup_token;
    };
}
