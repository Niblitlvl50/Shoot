
#include "Network/ClientManager.h"
#include "Network/NetworkMessage.h"
#include "Network/RemoteConnection.h"
#include "GameConfig.h"

#include "EventHandler/EventHandler.h"
#include "System/Network.h"
#include "System/System.h"
#include "System/Hash.h"

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
    const ClientStateMachine::StateTable& state_table = {
        ClientStateMachine::MakeState(ClientStatus::DISCONNECTED,   &ClientManager::ToDisconnected, this),
        ClientStateMachine::MakeState(ClientStatus::SEARCHING,      &ClientManager::ToSearching,    &ClientManager::Searching, this),
        ClientStateMachine::MakeState(ClientStatus::FOUND_SERVER,   &ClientManager::ToFoundServer,  this),
        ClientStateMachine::MakeState(ClientStatus::CONNECTED,      &ClientManager::ToConnected,    &ClientManager::Connected, this),
        ClientStateMachine::MakeState(ClientStatus::FAILED,         &ClientManager::ToFailed,       &ClientManager::Failed, this),
    };

    m_states.SetStateTableAndState(state_table, ClientStatus::DISCONNECTED);
}

ClientManager::~ClientManager()
{
    m_event_handler->RemoveListener(m_server_beacon_token);
    m_event_handler->RemoveListener(m_server_quit_token);
    m_event_handler->RemoveListener(m_connect_accepted_token);
    m_event_handler->RemoveListener(m_ping_token);
}

void ClientManager::StartClient()
{
    using namespace std::placeholders;
    const std::function<mono::EventResult (const ServerBeaconMessage&)> server_beacon_func = std::bind(&ClientManager::HandleServerBeacon, this, _1);
    const std::function<mono::EventResult (const ServerQuitMessage&)> server_quit_func = std::bind(&ClientManager::HandleServerQuit, this, _1);
    const std::function<mono::EventResult (const ConnectAcceptedMessage&)> connect_accepted_func = std::bind(&ClientManager::HandleConnectAccepted, this, _1);
    const std::function<mono::EventResult (const PingMessage&)> ping_func = std::bind(&ClientManager::HandlePing, this, _1);

    m_server_beacon_token = m_event_handler->AddListener(server_beacon_func);
    m_server_quit_token = m_event_handler->AddListener(server_quit_func);
    m_connect_accepted_token = m_event_handler->AddListener(connect_accepted_func);
    m_ping_token = m_event_handler->AddListener(ping_func);

    m_states.TransitionTo(ClientStatus::SEARCHING);
}

void ClientManager::Disconnect()
{
    if(m_states.ActiveState() == ClientStatus::CONNECTED)
    {
        NetworkMessage message;
        message.payload = SerializeMessage(DisconnectMessage());
        SendMessage(message);
    }

    m_states.TransitionTo(ClientStatus::DISCONNECTED);
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

    if(m_states.ActiveState() == ClientStatus::CONNECTED)
        info.stats = m_remote_connection->GetConnectionStats();

    info.additional_info.push_back("client: " + network::AddressToString(m_client_address));
    info.additional_info.push_back(std::string("status: ") + ClientStatusToString(GetConnectionStatus()));
    info.additional_info.push_back("");
    info.additional_info.push_back("client time: " + std::to_string(m_client_time));
    info.additional_info.push_back("server time: " + std::to_string(m_server_time));
    info.additional_info.push_back("predicted time: " + std::to_string(m_server_time_predicted));
    info.additional_info.push_back("");
    info.additional_info.push_back("server round trip: " + std::to_string(m_server_ping));

    return info;
}

uint32_t ClientManager::Id() const
{
    return hash::Hash(Name());
}

const char* ClientManager::Name() const
{
    return "clientmanager";
}

void ClientManager::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
    m_dispatcher.Update(update_context);

    m_client_time = update_context.timestamp;
    m_server_time_predicted += update_context.delta_ms;
}

mono::EventResult ClientManager::HandleServerBeacon(const ServerBeaconMessage& message)
{
    if(m_states.ActiveState() == ClientStatus::SEARCHING)
    {
        m_server_address = message.sender;
        m_states.TransitionTo(ClientStatus::FOUND_SERVER);
    }

    return mono::EventResult::HANDLED;
}

mono::EventResult ClientManager::HandleServerQuit(const ServerQuitMessage& message)
{
    System::Log("ClientManager|Server Quit!");
    m_states.TransitionTo(ClientStatus::DISCONNECTED);
    return mono::EventResult::HANDLED;
}

mono::EventResult ClientManager::HandleConnectAccepted(const ConnectAcceptedMessage& message)
{
    m_states.TransitionTo(ClientStatus::CONNECTED);
    return mono::EventResult::HANDLED;
}

mono::EventResult ClientManager::HandlePing(const PingMessage& message)
{
    m_server_ping = System::GetMilliseconds() - message.local_time;
    m_server_time = message.server_time;
    m_server_time_predicted = message.server_time;
    return mono::EventResult::PASS_ON;
}

void ClientManager::ToSearching()
{
    System::Log("ClientManager|Searching for server");
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
    System::Log("ClientManager|Found server at %s", network::AddressToString(m_server_address).c_str());

    NetworkMessage message;
    message.payload = SerializeMessage(ConnectMessage());
    SendMessage(message);
}

void ClientManager::ToConnected()
{
    System::Log("ClientManager|Server accepted connection");
}

void ClientManager::ToDisconnected()
{
    System::Log("ClientManager|Disconnected");
    m_remote_connection = nullptr;
}

void ClientManager::ToFailed()
{
    System::Log("ClientManager|Failed to find a server");
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
