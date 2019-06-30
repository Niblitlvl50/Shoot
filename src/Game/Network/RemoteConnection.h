
#pragma once

#include "NetworkMessage.h"
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
        uint32_t GetTotalSent() const;
        uint32_t GetTotalReceived() const;

    private:

        bool m_stop;
        std::thread m_comm_thread;

        struct OutgoingMessages
        {
            std::mutex message_mutex;
            std::vector<NetworkMessage> unhandled_messages;
        } m_messages;

        uint32_t m_total_sent;
        uint32_t m_total_received;
    };
}
