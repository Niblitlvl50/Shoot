
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"

#include <vector>
#include <memory>
#include <functional>

namespace System
{
    struct ControllerState;
}

namespace game
{
    class PlayerDaemon
    {
    public:

        PlayerDaemon(
            mono::ICameraPtr camera, const std::vector<math::Vector>& player_points, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~PlayerDaemon();

        void SpawnPlayer1();
        void SpawnPlayer2();

    private:

        using DestroyedCallback = std::function<void (uint32_t id)>;
        void SpawnPlayer(struct PlayerInfo* player_info, const System::ControllerState& controller, DestroyedCallback destroyed_callback);

        bool OnControllerAdded(const event::ControllerAddedEvent& event);
        bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

        mono::ICameraPtr m_camera;
        const std::vector<math::Vector> m_player_points;
        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;
    };
}
