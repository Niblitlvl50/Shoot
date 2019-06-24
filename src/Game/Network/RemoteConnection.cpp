
#include "RemoteConnection.h"
#include "MessageDispatcher.h"

#include <algorithm>

using namespace game;

RemoteConnection::RemoteConnection(MessageDispatcher* dispatcher, network::ISocketPtr socket)
    : m_stop(false)
{
    const auto comm_func = []
        (network::ISocketPtr socket, OutgoingMessages* out_messages, MessageDispatcher* dispatcher, bool& stop) {
        
        NetworkMessage message;
        message.payload.resize(1024, '\0');

        while(!stop)
        {
            bool sent_messages = false;
            bool received_messages = false;

            {
                std::lock_guard<std::mutex> lock(out_messages->message_mutex);
                sent_messages = !out_messages->unhandled_messages.empty();
                for(const NetworkMessage& message : out_messages->unhandled_messages)
                    socket->Send(message.payload, message.address);

                out_messages->unhandled_messages.clear();
            }
            
            {
                std::fill(message.payload.begin(), message.payload.begin() + message.payload.size(), '\0');
                const bool got_data = socket->Receive(message.payload, &message.address);
                if(got_data)
                {
                    dispatcher->PushNewMessage(message);
                    received_messages = true;
                }
            }

            if(!sent_messages && !received_messages)
                std::this_thread::yield();
        }
    };

    m_comm_thread = std::thread(comm_func, std::move(socket), &m_messages, dispatcher, std::ref(m_stop));
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_comm_thread.join();
}

void RemoteConnection::SendMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_messages.message_mutex);
    m_messages.unhandled_messages.push_back(message);
}
