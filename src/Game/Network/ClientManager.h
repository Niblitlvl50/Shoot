
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"
#include "EventHandler/EventToken.h"

#include "Network/INetworkPipe.h"
#include "Network/ClientStatus.h"
#include "Network/RemoteConnection.h"
#include "Network/MessageDispatcher.h"
#include "StateMachine.h"

#include <cstdint>

namespace game
{
    struct Config;

    struct ServerBeaconMessage;
    struct ServerQuitMessage;
    struct ConnectAcceptedMessage;

    class ClientManager : public mono::IUpdatable, public INetworkPipe
    {
    public:

        ClientManager(mono::EventHandler* event_handler, const game::Config* game_config);
        ~ClientManager();

        void SendMessage(const struct NetworkMessage& message) override;
        void SendMessageTo(const NetworkMessage& message, const network::Address& address) override;

        void Disconnect();

        ClientStatus GetConnectionStatus() const;

    private:

        void doUpdate(const mono::UpdateContext& update_context) override;

        bool HandleServerBeacon(const ServerBeaconMessage& message);
        bool HandleServerQuit(const ServerQuitMessage& message);
        bool HandleConnectAccepted(const ConnectAcceptedMessage& message);

        void ToSearching();
        void ToFoundServer();
        void ToConnected();
        void ToFailed();

        void Searching(const mono::UpdateContext& update_context);
        void Connected(const mono::UpdateContext& update_context);
        void Failed(const mono::UpdateContext& update_context);

        mono::EventHandler* m_event_handler;
        const game::Config* m_game_config;
        MessageDispatcher m_dispatcher;
        std::unique_ptr<RemoteConnection> m_remote_connection;

        int m_socket_port;
        uint32_t m_search_timer;
        uint32_t m_failed_timer;

        using ClientStateMachine = StateMachine<ClientStatus, const mono::UpdateContext&>;
        ClientStateMachine m_states;

        mono::EventToken<ServerBeaconMessage> m_server_beacon_token;
        mono::EventToken<ServerQuitMessage> m_server_quit_token;
        mono::EventToken<ConnectAcceptedMessage> m_connect_accepted_token;

        network::Address m_server_address;
    };
}
