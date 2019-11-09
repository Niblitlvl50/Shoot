
#include "ServerManager.h"
#include "EventHandler/EventHandler.h"
#include "RemoteConnection.h"
#include "GameConfig.h"
#include "Events/PlayerConnectedEvent.h"

#include "System/Network.h"
#include "System/System.h"

#include <functional>
#include <thread>

using namespace game;

ServerManager::ServerManager(mono::EventHandler* event_handler, const game::Config* game_config)
    : m_event_handler(event_handler)
    , m_game_config(game_config)
    , m_dispatcher(event_handler)
    , m_beacon_timer(0)
{
    PrintNetworkMessageSize();

    using namespace std::placeholders;

    const std::function<bool (const PingMessage&)> ping_func = std::bind(&ServerManager::HandlePingMessage, this, _1);
    const std::function<bool (const ConnectMessage&)> connect_func = std::bind(&ServerManager::HandleConnectMessage, this, _1);
    const std::function<bool (const DisconnectMessage&)> disconnect_func = std::bind(&ServerManager::HandleDisconnectMessage, this, _1);
    const std::function<bool (const HeartBeatMessage&)> heartbeat_func = std::bind(&ServerManager::HandleHeartBeatMessage, this, _1);
    const std::function<bool (const ViewportMessage&)> viewport_func = std::bind(&ServerManager::HandleViewportMessage, this, _1);

    m_ping_func_token = m_event_handler->AddListener(ping_func);
    m_connect_token = m_event_handler->AddListener(connect_func);
    m_disconnect_token = m_event_handler->AddListener(disconnect_func);
    m_heartbeat_token = m_event_handler->AddListener(heartbeat_func);
    m_viewport_token = m_event_handler->AddListener(viewport_func);

    network::ISocketPtr socket;
    if(game_config->use_port_range)
    {
        socket = network::CreateUDPSocket(network::SocketType::BLOCKING);
        m_broadcast_address = network::GetBroadcastAddress(socket->Port());
    }
    else
    {
        socket = network::CreateUDPSocket(network::SocketType::BLOCKING, game_config->server_port);
        m_broadcast_address = network::GetBroadcastAddress(game_config->client_port);
    }

    m_server_address = network::GetBroadcastAddress(socket->Port());
    m_remote_connection = std::make_unique<RemoteConnection>(&m_dispatcher, std::move(socket));
}

ServerManager::~ServerManager()
{
    QuitServer();

    m_event_handler->RemoveListener(m_ping_func_token);
    m_event_handler->RemoveListener(m_connect_token);
    m_event_handler->RemoveListener(m_disconnect_token);
    m_event_handler->RemoveListener(m_heartbeat_token);
    m_event_handler->RemoveListener(m_viewport_token);
}

void ServerManager::SendMessage(const NetworkMessage& message)
{
    m_remote_connection->SendData(message.payload, message.address);
}

void ServerManager::SendMessageTo(const NetworkMessage& message, const network::Address& address)
{
    m_remote_connection->SendData(message.payload, address);
}

ConnectionInfo ServerManager::GetConnectionInfo() const
{
    ConnectionInfo info;
    info.stats = m_remote_connection->GetConnectionStats();
    info.additional_info.push_back("server: " + network::AddressToString(m_server_address));

    info.additional_info.push_back("");
    info.additional_info.push_back("clients");
    for(const auto& pair : m_connected_clients)
        info.additional_info.push_back(network::AddressToString(pair.first));

    info.additional_info.push_back("");
    info.additional_info.push_back("server time: " + std::to_string(m_server_time));

    return info;
}

void ServerManager::QuitServer()
{
    NetworkMessage message;
    message.payload = SerializeMessage(ServerQuitMessage());
    SendMessage(message);
}

const std::unordered_map<network::Address, ClientData>& ServerManager::GetConnectedClients() const
{
    return m_connected_clients;
}

const ConnectionStats& ServerManager::GetConnectionStats() const
{
    return m_remote_connection->GetConnectionStats();
}

bool ServerManager::HandlePingMessage(const PingMessage& ping_message)
{
    PingMessage local_ping_message = ping_message;
    local_ping_message.server_time = m_server_time;

    NetworkMessage message;
    message.payload = SerializeMessage(local_ping_message);
    SendMessageTo(message, local_ping_message.sender);

    return true;
}

bool ServerManager::HandleConnectMessage(const ConnectMessage& message)
{
    ClientData client_data;
    client_data.heartbeat_timestamp = System::GetMilliseconds();

    const auto insert_result = m_connected_clients.insert(std::make_pair(message.sender, client_data));
    if(insert_result.second)
    {
        const std::string& address_string = network::AddressToString(message.sender);
        System::Log("ServerManager|Client connected: %s\n", address_string.c_str());

        NetworkMessage reply_message;
        reply_message.payload = SerializeMessage(ConnectAcceptedMessage());
        SendMessageTo(reply_message, message.sender);

        m_event_handler->DispatchEvent(PlayerConnectedEvent(message.sender.host));
    }
    else
    {
        System::Log("ServerManager|Client already in collection\n");
    }

    return false;
}

bool ServerManager::HandleDisconnectMessage(const DisconnectMessage& message)
{
    System::Log("ServerManager|Disconnect client\n");
    m_connected_clients.erase(message.sender);
    m_event_handler->DispatchEvent(PlayerDisconnectedEvent(message.sender.host));

    return false;
}

bool ServerManager::HandleHeartBeatMessage(const HeartBeatMessage& message)
{
    auto client_it = m_connected_clients.find(message.sender);
    if(client_it != m_connected_clients.end())
        client_it->second.heartbeat_timestamp = System::GetMilliseconds();
    else
        System::Log("ServerManager|Client not found in collection\n");

    return false;
}

bool ServerManager::HandleViewportMessage(const ViewportMessage& message)
{
    auto client_it = m_connected_clients.find(message.sender);
    if(client_it != m_connected_clients.end())
        client_it->second.viewport = message.viewport;
    else
        System::Log("ServerManager|Client not found in collection\n");

    return false;
}

void ServerManager::PurgeZombieClients()
{
    constexpr uint32_t client_timeout = 5000;
    const uint32_t current_time = System::GetMilliseconds();

    std::vector<network::Address> dead_clients;

    for(const auto& pair : m_connected_clients)
    {
        const uint32_t heartbeat_delta = current_time - pair.second.heartbeat_timestamp;
        if(heartbeat_delta > client_timeout)
            dead_clients.push_back(pair.first);
    }

    for(const auto& key : dead_clients)
    {
        System::Log("ServerManager|Purging client '%s'\n", network::AddressToString(key).c_str());
        m_connected_clients.erase(key);
        m_event_handler->DispatchEvent(PlayerDisconnectedEvent(key.host));
    }
}

void ServerManager::doUpdate(const mono::UpdateContext& update_context)
{
    m_server_time = update_context.total_time;

    PurgeZombieClients();
    m_dispatcher.doUpdate(update_context);

    m_beacon_timer += update_context.delta_ms;

    if(m_beacon_timer >= 500)
    {
        ServerBeaconMessage beacon_message;
        beacon_message.sender = m_server_address;

        NetworkMessage message;
        message.payload = SerializeMessage(beacon_message);
        SendMessageTo(message, m_broadcast_address);

        m_beacon_timer = 0;
    }
}
