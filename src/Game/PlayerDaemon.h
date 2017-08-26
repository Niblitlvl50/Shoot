
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderPtrFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include <memory>

namespace game
{
    class Shuttle;

    class PlayerDaemon
    {
    public:

        PlayerDaemon(mono::EventHandler& event_handler, mono::IPhysicsZone* zone);
        ~PlayerDaemon();

        void SetCamera(const mono::ICameraPtr& camera);

        bool OnControllerAdded(const event::ControllerAddedEvent& event);
        bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

        mono::EventHandler& m_event_handler;
        mono::IPhysicsZone* m_zone;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        int m_player_one_id;
        int m_player_two_id;
        std::shared_ptr<Shuttle> m_player_one;
        std::shared_ptr<Shuttle> m_player_two;
        
        mono::ICameraPtr m_camera;
    };
}
