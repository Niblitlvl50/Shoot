
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "System/System.h"
#include "System/Network.h"
#include "StateMachine.h"

#include "Events/GameEventFuncFwd.h"
#include "AIKnowledge.h"

#include <vector>
#include <unordered_map>

namespace game
{
    class GameCamera;
    class INetworkPipe;
    struct RemoteInputMessage;
    struct ClientPlayerSpawned;

    enum class PlayerMetaState
    {
        NONE,
        SPAWNED,
        DEAD
    };

    class PlayerDaemon
    {
    public:

        PlayerDaemon(
            GameCamera* game_camera,
            INetworkPipe* remote_connection,
            mono::SystemContext* system_context,
            mono::EventHandler& event_handler);
        ~PlayerDaemon();

        void SpawnPlayer1();
        void SpawnPlayer2();

        std::vector<uint32_t> GetPlayerIds() const;

    private:

        mono::EventResult OnControllerAdded(const event::ControllerAddedEvent& event);
        mono::EventResult OnControllerRemoved(const event::ControllerRemovedEvent& event);
        mono::EventResult PlayerConnected(const PlayerConnectedEvent& event);
        mono::EventResult PlayerDisconnected(const PlayerDisconnectedEvent& event);
        mono::EventResult RemoteInput(const RemoteInputMessage& event);
        mono::EventResult PLayerScore(const ScoreEvent& event);

        GameCamera* m_game_camera;
        INetworkPipe* m_remote_connection;
        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;

        PlayerMetaState m_player_state;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;
        mono::EventToken<PlayerConnectedEvent> m_player_connected_token;
        mono::EventToken<PlayerDisconnectedEvent> m_player_disconnected_token;
        mono::EventToken<RemoteInputMessage> m_remote_input_token;
        mono::EventToken<ScoreEvent> m_score_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;

        struct RemotePlayerData
        {
            PlayerInfo player_info;
            System::ControllerState controller_state;
        };
        std::unordered_map<network::Address, RemotePlayerData> m_remote_players;
    };

    class ClientPlayerDaemon
    {
    public:

        ClientPlayerDaemon(GameCamera* game_camera, mono::EventHandler& event_handler);
        ~ClientPlayerDaemon();

        void SpawnPlayer1();

        mono::EventResult OnControllerAdded(const event::ControllerAddedEvent& event);
        mono::EventResult OnControllerRemoved(const event::ControllerRemovedEvent& event);
        mono::EventResult ClientSpawned(const ClientPlayerSpawned& message);

        GameCamera* m_game_camera;
        mono::EventHandler& m_event_handler;
        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;
        mono::EventToken<ClientPlayerSpawned> m_client_spawned_token;

        int m_player_one_controller_id = -1;
    };
}
