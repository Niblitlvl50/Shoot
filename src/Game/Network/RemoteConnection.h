
#pragma once

#include "NetworkMessage.h"
#include "ConnectionStats.h"
#include "System/Network.h"
#include <thread>
#include <mutex>
#include <vector>

namespace game
{
    class RemoteConnection
    {
    public:

        RemoteConnection(class MessageDispatcher* dispatcher, network::ISocketPtr socket);
        ~RemoteConnection();

        void SendMessage(const NetworkMessage& message);
        const ConnectionStats& GetConnectionStats() const;

        struct OutgoingMessages
        {
            std::mutex message_mutex;
            std::vector<NetworkMessage> unhandled_messages;
        };

    private:

        bool m_stop;
        network::ISocketPtr m_socket;
        std::thread m_comm_thread;
        std::thread m_send_thread;

        OutgoingMessages m_messages;
        ConnectionStats m_stats;
    };
}
