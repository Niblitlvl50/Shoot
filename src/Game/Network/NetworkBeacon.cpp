
#include "NetworkBeacon.h"

#include "System/Network.h"
#include "System/System.h"

using namespace game;

NetworkBeacon::NetworkBeacon()
    : m_broadcast(true)
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
    const auto broadcast_func = [](const bool& broadcast) {
        //Network::ISocketPtr broadcast_socket = Network::OpenBroadcastSocket(17776, false);
        Network::ISocketPtr broadcast_socket = Network::OpenLoopbackSocket(17776, false);
        if(!broadcast_socket)
            return;

        const std::vector<byte> data = { 'm', 'o', 'n', 'o', '1' };

        while(broadcast)
        {
            broadcast_socket->Send(data);
            System::Sleep(1000);
        }
    };

    m_broadcast_thread = std::thread(broadcast_func, std::ref(m_broadcast));
}

void NetworkBeacon::Stop()
{
    m_broadcast = false;
}
