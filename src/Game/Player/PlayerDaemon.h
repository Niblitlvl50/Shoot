
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
            mono::ICameraPtr camera, const std::vector<math::Vector>& player_points, mono::EventHandler& event_handler);
        ~PlayerDaemon();

        void SpawnPlayer1();
        void SpawnPlayer2();

    private:

        bool OnControllerAdded(const event::ControllerAddedEvent& event);
        bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

        mono::ICameraPtr m_camera;
        const std::vector<math::Vector> m_player_points;
        mono::EventHandler& m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;
        std::shared_ptr<Shuttle> m_player_one;
        std::shared_ptr<Shuttle> m_player_two;
    };
}
