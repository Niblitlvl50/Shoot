
#pragma once

#include "MonoFwd.h"
#include "Rendering/RenderPtrFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"

#include <vector>
#include <memory>

namespace game
{
    class Shuttle;

    class PlayerDaemon
    {
    public:

        PlayerDaemon(
            const std::vector<math::Vector>& player_points, mono::EventHandler& event_handler);
        ~PlayerDaemon();

        void SetCamera(const mono::ICameraPtr& camera);

    private:

        bool OnControllerAdded(const event::ControllerAddedEvent& event);
        bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

        const std::vector<math::Vector> m_player_points;
        mono::EventHandler& m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;
        std::shared_ptr<Shuttle> m_player_one;
        std::shared_ptr<Shuttle> m_player_two;
        
        mono::ICameraPtr m_camera;
    };
}
