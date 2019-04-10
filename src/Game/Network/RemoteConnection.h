
#pragma once

#include "NetworkMessage.h"
#include "System/Network.h"
#include <thread>
#include <mutex>

namespace game
{
    class RemoteConnection
    {
    public:

        RemoteConnection(
            class MessageDispatcher* dispatcher, network::ISocketPtr in_socket, network::ISocketPtr out_socket, const network::Address& target);
        ~RemoteConnection();

        void SendMessage(const NetworkMessage& message);

    private:

        bool m_stop;
        std::thread m_incomming_thread;
        std::thread m_outgoing_thread;

        struct OutgoingMessages
        {
            std::mutex message_mutex;
            std::vector<NetworkMessage> unhandled_messages;
        };

        OutgoingMessages m_messages;
    };
}
