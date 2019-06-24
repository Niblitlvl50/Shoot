
#include "ServerManager.h"
#include "EventHandler/EventHandler.h"
#include "RemoteConnection.h"
#include "GameConfig.h"
#include "Events/PlayerConnectedEvent.h"

#include "System/Network.h"

#include <functional>
#include <thread>

using namespace game;

ServerManager::ServerManager(mono::EventHandler* event_handler, const game::Config* game_config)
    : m_event_handler(event_handler)
    , m_game_config(game_config)
    , m_dispatcher(*event_handler)
    , m_beacon_timer(0)
{
    using namespace std::placeholders;

    const std::function<bool (const PingMessage&)> ping_func = std::bind(&ServerManager::HandlePingMessage, this, _1);
    const std::function<bool (const ConnectMessage&)> connect_func = std::bind(&ServerManager::HandleConnectMessage, this, _1);
    const std::function<bool (const DisconnectMessage&)> disconnect_func = std::bind(&ServerManager::HandleDisconnectMessage, this, _1);
    const std::function<bool (const HeartBeatMessage&)> heartbeat_func = std::bind(&ServerManager::HandleHeartBeatMessage, this, _1);

    m_ping_func_token = m_event_handler->AddListener(ping_func);
    m_connect_token = m_event_handler->AddListener(connect_func);
    m_disconnect_token = m_event_handler->AddListener(disconnect_func);
    m_heartbeat_token = m_event_handler->AddListener(heartbeat_func);

    network::ISocketPtr socket;
    if(game_config->server_port == 0)
        socket = network::CreateUDPSocket(network::SocketType::NON_BLOCKING);
    else
        socket = network::CreateUDPSocket(network::SocketType::NON_BLOCKING, game_config->server_port);

    //m_out_address = network::GetBroadcastAddress(socket->Port());
    m_out_address = network::GetBroadcastAddress(game_config->client_port);
    m_remote_connection = std::make_unique<RemoteConnection>(&m_dispatcher, std::move(socket));
}

ServerManager::~ServerManager()
{
    QuitServer();
    std::this_thread::yield();

    m_event_handler->RemoveListener(m_ping_func_token);
    m_event_handler->RemoveListener(m_connect_token);
    m_event_handler->RemoveListener(m_disconnect_token);
    m_event_handler->RemoveListener(m_heartbeat_token);
}

void ServerManager::SendMessage(const NetworkMessage& message)
{
    NetworkMessage out_message = message;
    out_message.address = m_out_address;
    m_remote_connection->SendMessage(out_message);
}

void ServerManager::SendMessageTo(const NetworkMessage& message, const network::Address& address)
{

}

void ServerManager::QuitServer()
{
    NetworkMessage message;
    message.payload = SerializeMessage(ServerQuitMessage());
    SendMessage(message);
}

std::vector<ClientData> ServerManager::GetConnectedClients() const
{
    std::vector<ClientData> connected_clients;
    connected_clients.reserve(m_connected_clients.size());

    for(const auto& pair : m_connected_clients)
        connected_clients.push_back(pair.second);

    return connected_clients;
}

bool ServerManager::HandlePingMessage(const PingMessage& ping_message)
{
    NetworkMessage message;
    //message.address = ping_message.sender_address;
    message.payload = SerializeMessage(ping_message);
    SendMessage(message);

    return true;
}

bool ServerManager::HandleConnectMessage(const ConnectMessage& message)
{
    const std::string& address_string = network::AddressToString(message.sender_address);
    std::printf("connected client: %s\n", address_string.c_str());

    ClientData client_data;
    client_data.address = message.sender_address;
    client_data.heartbeat_timestamp = System::GetMilliseconds();

    const auto insert_result = m_connected_clients.insert(std::make_pair(message.sender_address, client_data));
    if(insert_result.second)
    {
        std::printf("ServerManager|Client connected!\n");

        NetworkMessage reply_message;
        //reply_message.address = message.sender_address;
        reply_message.payload = SerializeMessage(ConnectAcceptedMessage());
        SendMessage(reply_message);

        m_event_handler->DispatchEvent(PlayerConnectedEvent(message.sender_address.host));
    }
    else
    {
        std::printf("ServerManager|Client already in collection\n");
    }

    return false;
}

bool ServerManager::HandleDisconnectMessage(const DisconnectMessage& message)
{
    std::printf("ServerManager|Disconnect client\n");
    m_connected_clients.erase(message.sender_address);
    m_event_handler->DispatchEvent(PlayerDisconnectedEvent(message.sender_address.host));

    return false;
}

bool ServerManager::HandleHeartBeatMessage(const HeartBeatMessage& message)
{
    auto client_it = m_connected_clients.find(message.sender_address);
    if(client_it != m_connected_clients.end())
        client_it->second.heartbeat_timestamp = System::GetMilliseconds();
    else
        std::printf("ServerManager|Client not found in collection\n");

    return false;
}

void ServerManager::PurgeZombieClients()
{
    constexpr uint32_t client_timeout = 2000;
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
        std::printf("Network|Purging client '%s'\n", network::AddressToString(key).c_str());
        m_connected_clients.erase(key);

        m_event_handler->DispatchEvent(PlayerDisconnectedEvent(key.host));
    }
}

void ServerManager::doUpdate(const mono::UpdateContext& update_context)
{
    PurgeZombieClients();
    m_dispatcher.doUpdate(update_context);

    m_beacon_timer += update_context.delta_ms;

    if(m_beacon_timer >= 100)
    {
        NetworkMessage message;
        message.payload = SerializeMessage(ServerBeaconMessage());
        SendMessage(message);

        m_beacon_timer = 0;
    }
}