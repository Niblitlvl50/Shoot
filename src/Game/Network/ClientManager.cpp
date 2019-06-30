
#include "Network/ClientManager.h"
#include "Network/NetworkMessage.h"
#include "Network/RemoteConnection.h"
#include "GameConfig.h"

#include "EventHandler/EventHandler.h"
#include "System/Network.h"

using namespace game;

ClientManager::ClientManager(mono::EventHandler* event_handler, const game::Config* game_config)
    : m_event_handler(event_handler)
    , m_game_config(game_config)
    , m_dispatcher(*event_handler)
    , m_socket_port(game_config->port_range_start)
{
    using namespace std::placeholders;

    const std::unordered_map<ClientStatus, ClientStateMachine::State>& state_table = {
        { ClientStatus::DISCONNECTED,   { nullptr, nullptr} },
        { ClientStatus::SEARCHING,      { std::bind(&ClientManager::ToSearching,   this), std::bind(&ClientManager::Searching, this, _1) } },
        { ClientStatus::FOUND_SERVER,   { std::bind(&ClientManager::ToFoundServer, this), nullptr } },
        { ClientStatus::CONNECTED,      { std::bind(&ClientManager::ToConnected,   this), std::bind(&ClientManager::Connected, this, _1) } },
        { ClientStatus::FAILED,         { std::bind(&ClientManager::ToFailed,      this), std::bind(&ClientManager::Failed,    this, _1) } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(ClientStatus::SEARCHING);

    const std::function<bool (const ServerBeaconMessage&)> server_beacon_func = std::bind(&ClientManager::HandleServerBeacon, this, _1);
    const std::function<bool (const ServerQuitMessage&)> server_quit_func = std::bind(&ClientManager::HandleServerQuit, this, _1);
    const std::function<bool (const ConnectAcceptedMessage&)> connect_accepted_func = std::bind(&ClientManager::HandleConnectAccepted, this, _1);

    m_server_beacon_token = m_event_handler->AddListener(server_beacon_func);
    m_server_quit_token = m_event_handler->AddListener(server_quit_func);
    m_connect_accepted_token = m_event_handler->AddListener(connect_accepted_func);
}

ClientManager::~ClientManager()
{
    Disconnect();
    std::this_thread::yield();

    m_event_handler->RemoveListener(m_server_beacon_token);
    m_event_handler->RemoveListener(m_server_quit_token);
    m_event_handler->RemoveListener(m_connect_accepted_token);
}

ClientStatus ClientManager::GetConnectionStatus() const
{
    return m_states.ActiveState();
}

uint32_t ClientManager::GetTotalSent() const
{
    return m_remote_connection->GetTotalSent();
}

uint32_t ClientManager::GetTotalReceived() const
{
    return m_remote_connection->GetTotalReceived();
}

void ClientManager::SendMessage(const NetworkMessage& message)
{
    // Nice! I dont have to do a copy!
    const_cast<NetworkMessage&>(message).address = m_server_address;
    m_remote_connection->SendMessage(message);
}

void ClientManager::SendMessageTo(const NetworkMessage& message, const network::Address& address)
{
    // Nice! I dont have to do a copy!
    const_cast<NetworkMessage&>(message).address = address;
    m_remote_connection->SendMessage(message);
}

void ClientManager::Disconnect()
{
    NetworkMessage message;
    message.payload = SerializeMessage(DisconnectMessage());
    SendMessage(message);

    m_states.TransitionTo(ClientStatus::DISCONNECTED);
}

void ClientManager::doUpdate(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_dispatcher.doUpdate(update_context);
}

bool ClientManager::HandleServerBeacon(const ServerBeaconMessage& message)
{
    if(m_states.ActiveState() == ClientStatus::SEARCHING)
    {
        std::printf("Server beacond %s\n", network::AddressToString(message.header.sender).c_str());
        m_server_address = message.header.sender;
        m_states.TransitionTo(ClientStatus::FOUND_SERVER);
    }

    return true;
}

bool ClientManager::HandleServerQuit(const ServerQuitMessage& message)
{
    m_states.TransitionTo(ClientStatus::DISCONNECTED);
    return true;
}

bool ClientManager::HandleConnectAccepted(const ConnectAcceptedMessage& message)
{
    m_states.TransitionTo(ClientStatus::CONNECTED);
    return true;
}

void ClientManager::ToSearching()
{
    std::printf("network|Searching for server\n");
    m_search_timer = 0;

    network::ISocketPtr socket;
    do
    {
        unsigned short client_port = m_game_config->client_port;
        if(client_port == 0)
        {
            client_port = m_socket_port++;
            if(m_socket_port > m_game_config->port_range_end)
                m_socket_port = m_game_config->port_range_start;
        }

        socket = network::CreateUDPSocket(network::SocketType::NON_BLOCKING, client_port);
    } while(!socket);

    m_remote_connection = std::make_unique<RemoteConnection>(&m_dispatcher, std::move(socket));
}

void ClientManager::ToFoundServer()
{
    std::printf("network|Found server\n");

    NetworkMessage message;
    message.payload = SerializeMessage(ConnectMessage());
    SendMessage(message);
}

void ClientManager::ToConnected()
{
    std::printf("network|Server accepted connection\n");
}

void ClientManager::ToFailed()
{
    std::printf("network|Failed to found a server\n");
    m_failed_timer = 0;
}

void ClientManager::Searching(const mono::UpdateContext& update_context)
{
    m_search_timer += update_context.delta_ms;
    if(m_search_timer >= 15000)
        m_states.TransitionTo(ClientStatus::FAILED);
}

void ClientManager::Connected(const mono::UpdateContext& update_context)
{
    const bool is_thirtieth_frame = (update_context.frame_count % 30) == 0;
    if(is_thirtieth_frame)
    {
        NetworkMessage message;
        message.payload = SerializeMessage(HeartBeatMessage());
        SendMessage(message);
    }

    const bool is_sixtieth_frame = (update_context.frame_count % 60) == 0;
    if(is_sixtieth_frame)
    {
        PingMessage ping_message;
        ping_message.local_time_stamp = System::GetMilliseconds();

        NetworkMessage message;
        message.payload = SerializeMessage(ping_message);
        SendMessage(message);
    }
}

void ClientManager::Failed(const mono::UpdateContext& update_context)
{
    m_failed_timer += update_context.delta_ms;
    if(m_search_timer >= 500)
        m_states.TransitionTo(ClientStatus::SEARCHING);
}
