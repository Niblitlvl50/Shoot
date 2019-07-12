
#include "RemoteConnection.h"
#include "MessageDispatcher.h"

#include <algorithm>

using namespace game;

RemoteConnection::RemoteConnection(MessageDispatcher* dispatcher, network::ISocketPtr socket)
    : m_stop(false)
    , m_socket(std::move(socket))
{
    m_stats = { 0, 0, 0, 0 };

    const auto comm_func = []
        (network::ISocket* socket, MessageDispatcher* dispatcher, uint32_t& total_received, uint32_t& total_byte_received, bool& stop) {
        
        NetworkMessage message;
        message.payload.resize(NetworkMessageBufferSize);

        while(!stop)
        {
            std::fill(message.payload.begin(), message.payload.begin() + message.payload.size(), '\0');

            const int bytes_received = socket->Receive(message.payload, &message.address);
            if(bytes_received > 0)
            {
                dispatcher->PushNewMessage(message);
                total_byte_received += bytes_received;
                ++total_received;
            }
            else
            {
                std::this_thread::yield();
            }
        }
    };

    const auto send_func = []
        (network::ISocket* socket, OutgoingMessages* out_messages, uint32_t& total_sent, uint32_t& total_byte_sent, bool& stop) {

        while(!stop)
        {
            bool no_messages = true;

            {
                std::lock_guard<std::mutex> lock(out_messages->message_mutex);
                for(const NetworkMessage& message : out_messages->unhandled_messages)
                {
                    if(socket->Send(message.payload, message.address))
                    {
                        total_sent++;
                        total_byte_sent += message.payload.size();
                    }
                }

                no_messages = out_messages->unhandled_messages.empty();
                out_messages->unhandled_messages.clear();
            }

            if(no_messages)
                std::this_thread::yield();
        }
    };

    m_comm_thread = std::thread(
        comm_func, m_socket.get(), dispatcher, std::ref(m_stats.total_received), std::ref(m_stats.total_byte_received), std::ref(m_stop));

    m_send_thread = std::thread(
        send_func, m_socket.get(), &m_messages, std::ref(m_stats.total_sent), std::ref(m_stats.total_byte_sent), std::ref(m_stop));
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_comm_thread.join();
    m_send_thread.join();
}

void RemoteConnection::SendMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_messages.message_mutex);
    m_messages.unhandled_messages.push_back(message);
}

const ConnectionStats& RemoteConnection::GetConnectionStats() const
{
    return m_stats;
}
