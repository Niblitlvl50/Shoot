
#pragma once

#include "NetworkMessage.h"
#include <thread>
#include <mutex>

namespace game
{
    class RemoteConnection
    {
    public:

        RemoteConnection(class MessageDispatcher* dispatcher);
        ~RemoteConnection();

        void SendMessage(const NetworkMessage& message);

    private:

        bool m_stop;
        std::thread m_comm_thread;
        std::thread m_outgoing_thread;

        std::mutex m_message_mutex;
        std::vector<NetworkMessage> m_unhandled_messages;
    };
}
