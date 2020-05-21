
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "EventHandler/EventToken.h"
#include "System/Network.h"

#include "Network/INetworkPipe.h"
#include "Network/ClientStatus.h"
#include "Network/MessageDispatcher.h"
#include "StateMachine.h"

#include <cstdint>
#include <memory>

namespace game
{
    struct Config;

    struct ServerBeaconMessage;
    struct ServerQuitMessage;
    struct ConnectAcceptedMessage;
    struct PingMessage;

    class ClientManager : public mono::IUpdatable, public INetworkPipe
    {
    public:

        ClientManager(mono::EventHandler* event_handler, const game::Config* game_config);
        ~ClientManager();

        void SendMessage(const struct NetworkMessage& message) override;
        void SendMessageTo(const NetworkMessage& message, const network::Address& address) override;
        ConnectionInfo GetConnectionInfo() const override;

        void Disconnect();

        ClientStatus GetConnectionStatus() const;
        const struct ConnectionStats& GetConnectionStats() const;
        const network::Address& GetClientAddress() const;
        const network::Address& GetServerAddress() const;
        uint32_t GetServerPing() const;
        uint32_t GetServerTime() const;
        uint32_t GetServerTimePredicted() const;

    private:

        void Update(const mono::UpdateContext& update_context) override;

        mono::EventResult HandleServerBeacon(const ServerBeaconMessage& message);
        mono::EventResult HandleServerQuit(const ServerQuitMessage& message);
        mono::EventResult HandleConnectAccepted(const ConnectAcceptedMessage& message);
        mono::EventResult HandlePing(const PingMessage& message);

        void ToSearching();
        void ToFoundServer();
        void ToConnected();
        void ToDisconnected();
        void ToFailed();

        void Searching(const mono::UpdateContext& update_context);
        void Connected(const mono::UpdateContext& update_context);
        void Failed(const mono::UpdateContext& update_context);

        mono::EventHandler* m_event_handler;
        const game::Config* m_game_config;
        MessageDispatcher m_dispatcher;
        std::unique_ptr<class RemoteConnection> m_remote_connection;

        int m_socket_port;
        uint32_t m_search_timer;
        uint32_t m_failed_timer;

        using ClientStateMachine = StateMachine<ClientStatus, const mono::UpdateContext&>;
        ClientStateMachine m_states;

        mono::EventToken<ServerBeaconMessage> m_server_beacon_token;
        mono::EventToken<ServerQuitMessage> m_server_quit_token;
        mono::EventToken<ConnectAcceptedMessage> m_connect_accepted_token;
        mono::EventToken<PingMessage> m_ping_token;

        network::Address m_server_address;
        network::Address m_client_address;
        uint32_t m_server_ping;
        uint32_t m_server_time;
        uint32_t m_server_time_predicted;
        uint32_t m_client_time;
    };
}
