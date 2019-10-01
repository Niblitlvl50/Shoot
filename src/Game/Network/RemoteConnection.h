
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

        void SendData(const std::vector<byte>& data, const network::Address& target);
        void SendData(const std::vector<byte>& data, const std::vector<network::Address>& addresses);
        const ConnectionStats& GetConnectionStats() const;

        struct Message
        {
            std::vector<byte> payload;
            std::vector<network::Address> addresses;
        };

        struct OutgoingMessages
        {
            std::mutex message_mutex;
            std::vector<Message> unhandled_messages;
        };

    private:

        bool m_stop;
        network::ISocketPtr m_socket;
        std::thread m_receive_thread;
        std::thread m_send_thread;

        OutgoingMessages m_messages;
        ConnectionStats m_stats;

        uint32_t m_sequence_id;
    };
}
