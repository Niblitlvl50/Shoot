
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "Math/Vector.h"
#include "System/System.h"

#include "Events/GameEventFuncFwd.h"
#include "AIKnowledge.h"

#include <vector>
#include <memory>
#include <functional>
#include <unordered_map>

namespace System
{
    struct ControllerState;
}

namespace game
{
    struct RemoteInputMessage;

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

        bool PlayerConnected(const PlayerConnectedEvent& event);
        bool PlayerDisconnected(const PlayerDisconnectedEvent& event);
        bool RemoteInput(const RemoteInputMessage& event);

        mono::ICameraPtr m_camera;
        const std::vector<math::Vector> m_player_points;
        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        mono::EventToken<PlayerConnectedEvent> m_player_connected_token;
        mono::EventToken<PlayerDisconnectedEvent> m_player_disconnected_token;
        mono::EventToken<RemoteInputMessage> m_remote_input_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;

        struct RemotePlayerData
        {
            PlayerInfo player_info;
            System::ControllerState controller_state;
        };
        std::unordered_map<uint32_t, RemotePlayerData> m_remote_players;
    };

    class ClientPlayerDaemon
    {
    public:

        ClientPlayerDaemon(mono::EventHandler& event_handler);
        ~ClientPlayerDaemon();

        void SpawnPlayer1();

        bool OnControllerAdded(const event::ControllerAddedEvent& event);
        bool OnControllerRemoved(const event::ControllerRemovedEvent& event);

        mono::EventHandler& m_event_handler;
        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;

        int m_player_one_id = -1;
    };
}
