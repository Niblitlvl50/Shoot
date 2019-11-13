
#include "Network/ClientManager.h"
#include "Network/NetworkMessage.h"
#include "Network/RemoteConnection.h"
#include "GameConfig.h"

#include "EventHandler/EventHandler.h"
#include "System/Network.h"
#include "System/System.h"

using namespace game;

ClientManager::ClientManager(mono::EventHandler* event_handler, const game::Config* game_config)
    : m_event_handler(event_handler)
    , m_game_config(game_config)
    , m_dispatcher(event_handler)
    , m_socket_port(game_config->port_range_start)
    , m_server_ping(0)
    , m_server_time(0)
    , m_server_time_predicted(0)
{
    PrintNetworkMessageSize();

    using namespace std::placeholders;

    const std::function<bool (const ServerBeaconMessage&)> server_beacon_func = std::bind(&ClientManager::HandleServerBeacon, this, _1);
    const std::function<bool (const ServerQuitMessage&)> server_quit_func = std::bind(&ClientManager::HandleServerQuit, this, _1);
    const std::function<bool (const ConnectAcceptedMessage&)> connect_accepted_func = std::bind(&ClientManager::HandleConnectAccepted, this, _1);
    const std::function<bool (const PingMessage&)> ping_func = std::bind(&ClientManager::HandlePing, this, _1);

    m_server_beacon_token = m_event_handler->AddListener(server_beacon_func);
    m_server_quit_token = m_event_handler->AddListener(server_quit_func);
    m_connect_accepted_token = m_event_handler->AddListener(connect_accepted_func);
    m_ping_token = m_event_handler->AddListener(ping_func);

    const std::unordered_map<ClientStatus, ClientStateMachine::State>& state_table = {
        { ClientStatus::DISCONNECTED,   { std::bind(&ClientManager::ToDisconnected, this), nullptr} },
        { ClientStatus::SEARCHING,      { std::bind(&ClientManager::ToSearching,    this), std::bind(&ClientManager::Searching, this, _1) } },
        { ClientStatus::FOUND_SERVER,   { std::bind(&ClientManager::ToFoundServer,  this), nullptr } },
        { ClientStatus::CONNECTED,      { std::bind(&ClientManager::ToConnected,    this), std::bind(&ClientManager::Connected, this, _1) } },
        { ClientStatus::FAILED,         { std::bind(&ClientManager::ToFailed,       this), std::bind(&ClientManager::Failed,    this, _1) } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(ClientStatus::SEARCHING);
}

ClientManager::~ClientManager()
{
    Disconnect();

    m_event_handler->RemoveListener(m_server_beacon_token);
    m_event_handler->RemoveListener(m_server_quit_token);
    m_event_handler->RemoveListener(m_connect_accepted_token);
    m_event_handler->RemoveListener(m_ping_token);
}

ClientStatus ClientManager::GetConnectionStatus() const
{
    return m_states.ActiveState();
}

const ConnectionStats& ClientManager::GetConnectionStats() const
{
    return m_remote_connection->GetConnectionStats();
}

const network::Address& ClientManager::GetClientAddress() const
{
    return m_client_address;
}

const network::Address& ClientManager::GetServerAddress() const
{
    return m_server_address;
}

uint32_t ClientManager::GetServerPing() const
{
    return m_server_ping;
}

uint32_t ClientManager::GetServerTime() const
{
    return m_server_time;
}

uint32_t ClientManager::GetServerTimePredicted() const
{
    return m_server_time_predicted - m_game_config->client_time_offset;
}

void ClientManager::SendMessage(const NetworkMessage& message)
{
    m_remote_connection->SendData(message.payload, m_server_address);
}

void ClientManager::SendMessageTo(const NetworkMessage& message, const network::Address& address)
{
    m_remote_connection->SendData(message.payload, address);
}

ConnectionInfo ClientManager::GetConnectionInfo() const
{
    ConnectionInfo info;
    info.stats = m_remote_connection->GetConnectionStats();
    info.additional_info.push_back(network::AddressToString(m_client_address));
    info.additional_info.push_back(ClientStatusToString(GetConnectionStatus()));
    info.additional_info.push_back("client: " + std::to_string(m_client_time));
    info.additional_info.push_back("server: " + std::to_string(m_server_time));
    info.additional_info.push_back("predicted: " + std::to_string(m_server_time_predicted));

    if(GetConnectionStatus() == ClientStatus::CONNECTED)
        info.additional_info.push_back("ping: " + std::to_string(m_server_ping));

    return info;
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

    m_client_time = update_context.total_time;
    m_server_time_predicted += update_context.delta_ms;
}

bool ClientManager::HandleServerBeacon(const ServerBeaconMessage& message)
{
    if(m_states.ActiveState() == ClientStatus::SEARCHING)
    {
        m_server_address = message.sender;
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

bool ClientManager::HandlePing(const PingMessage& message)
{
    m_server_ping = System::GetMilliseconds() - message.local_time;
    m_server_time = message.server_time;
    m_server_time_predicted = message.server_time;
    return false;
}

void ClientManager::ToSearching()
{
    System::Log("network|Searching for server\n");
    m_search_timer = 0;

    m_remote_connection.reset();

    network::ISocketPtr socket;
    do
    {
        uint16_t client_port = m_game_config->client_port;
        if(m_game_config->use_port_range)
        {
            client_port = m_socket_port++;
            if(m_socket_port > m_game_config->port_range_end)
                m_socket_port = m_game_config->port_range_start;
        }

        socket = network::CreateUDPSocket(network::SocketType::BLOCKING, client_port);
    } while(!socket);

    m_client_address = network::MakeAddress(network::GetLocalhostName().c_str(), socket->Port());
    m_remote_connection = std::make_unique<RemoteConnection>(&m_dispatcher, std::move(socket));
}

void ClientManager::ToFoundServer()
{
    System::Log("network|Found server at %s\n", network::AddressToString(m_server_address).c_str());

    NetworkMessage message;
    message.payload = SerializeMessage(ConnectMessage());
    SendMessage(message);
}

void ClientManager::ToConnected()
{
    System::Log("network|Server accepted connection\n");
}

void ClientManager::ToDisconnected()
{
    System::Log("network|Server quit\n");
}

void ClientManager::ToFailed()
{
    System::Log("network|Failed to find a server\n");
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
    const bool is_tenth_frame = (update_context.frame_count % 30) == 0;
    if(is_tenth_frame)
    {
        NetworkMessage message;
        message.payload = SerializeMessage(HeartBeatMessage());
        SendMessage(message);
    }

    const bool is_fifth_frame = (update_context.frame_count % 15) == 0;
    if(is_fifth_frame)
    {
        PingMessage ping_message;
        ping_message.local_time = System::GetMilliseconds();
        ping_message.server_time = 0;

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
