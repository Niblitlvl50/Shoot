
#pragma once

#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "System/System.h"
#include "System/Network.h"

#include "Events/GameEventFuncFwd.h"
#include "AIKnowledge.h"

#include <vector>
#include <unordered_map>

namespace game
{
    class INetworkPipe;
    struct RemoteInputMessage;
    struct ClientPlayerSpawned;

    class PlayerDaemon
    {
    public:

        PlayerDaemon(
            INetworkPipe* remote_connection,
            mono::IEntityManager* entity_system,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const math::Vector& player_one_spawn);
        ~PlayerDaemon();

        void SpawnPlayer1();
        void SpawnPlayer2();

        std::vector<uint32_t> GetPlayerIds() const;

    private:

        mono::EventResult OnControllerAdded(const event::ControllerAddedEvent& event);
        mono::EventResult OnControllerRemoved(const event::ControllerRemovedEvent& event);
        mono::EventResult RemotePlayerConnected(const PlayerConnectedEvent& event);
        mono::EventResult RemotePlayerDisconnected(const PlayerDisconnectedEvent& event);
        mono::EventResult RemotePlayerInput(const RemoteInputMessage& event);
        mono::EventResult PlayerScore(const ScoreEvent& event);
        mono::EventResult OnSpawnPlayer(const SpawnPlayerEvent& event);

        class CameraSystem* m_camera_system;
        INetworkPipe* m_remote_connection;
        mono::IEntityManager* m_entity_system;
        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;

        mono::EventToken<event::ControllerAddedEvent> m_added_token;
        mono::EventToken<event::ControllerRemovedEvent> m_removed_token;
        mono::EventToken<PlayerConnectedEvent> m_player_connected_token;
        mono::EventToken<PlayerDisconnectedEvent> m_player_disconnected_token;
        mono::EventToken<RemoteInputMessage> m_remote_input_token;
        mono::EventToken<ScoreEvent> m_score_token;
        mono::EventToken<SpawnPlayerEvent> m_spawn_player_token;

        int m_player_one_id = -1;
        int m_player_two_id = -1;

        math::Vector m_player_one_spawn;
        math::Vector m_player_two_spawn;

        struct RemotePlayerData
        {
            PlayerInfo player_info;
            System::ControllerState controller_state;
        };
        std::unordered_map<network::Address, RemotePlayerData> m_remote_players;
    };
}
