
#pragma once

#include "MonoFwd.h"
#include "EventHandler/EventToken.h"

namespace event
{
    struct ControllerAddedEvent;
    struct ControllerRemovedEvent;
}

namespace game
{
    class Shuttle;
}

class ShuttleGamepadController
{
public:

    ShuttleGamepadController(game::Shuttle* shuttle, mono::EventHandler& event_handler);
    ~ShuttleGamepadController();

    void Update(unsigned int delta);

private:

    bool OnControllerAdded(const event::ControllerAddedEvent& event);
    bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

    game::Shuttle* m_shuttle;
    mono::EventHandler& m_event_handler;

    mono::EventToken<event::ControllerAddedEvent> m_added_token;
    mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

    int m_id = -1;
};
