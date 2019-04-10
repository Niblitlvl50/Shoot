
#include "NetworkBeacon.h"

#include "System/Network.h"
#include "System/System.h"

using namespace game;

NetworkBeacon::NetworkBeacon(int beacon_port)
    : m_beacon_port(beacon_port)
    , m_broadcast(false)
{
    Start();
}

NetworkBeacon::~NetworkBeacon()
{
    Stop();
    m_broadcast_thread.join();
}

void NetworkBeacon::Start()
{
    if(m_broadcast)
        return;

    const auto broadcast_func = [](int beacon_port, const bool& broadcast) {
        network::ISocketPtr broadcast_socket = network::OpenLoopbackSocket(beacon_port, false);
        if(!broadcast_socket)
            return;

        const std::vector<byte> data = { 'm', 'o', 'n', 'o', '1' };

        while(broadcast)
        {
            broadcast_socket->Send(data);
            System::Sleep(250);
        }
    };

    m_broadcast = true;
    m_broadcast_thread = std::thread(broadcast_func, m_beacon_port, std::ref(m_broadcast));
}

void NetworkBeacon::Stop()
{
    m_broadcast = false;
}
