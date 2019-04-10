
#include "RemoteConnection.h"
#include "MessageDispatcher.h"

#include "System/Network.h"
#include "System/System.h"

#include <algorithm>

using namespace game;

RemoteConnection::RemoteConnection(
    MessageDispatcher* dispatcher, network::ISocketPtr in_socket, network::ISocketPtr out_socket, const network::Address& target)
    : m_stop(false)
{
    const auto comm_func = [](network::ISocketPtr socket, MessageDispatcher* dispatcher, const bool& stop) {
        network::Address address;
        std::vector<byte> data(1024, '\0');
        while(!stop)
        {
            std::fill(data.begin(), data.begin() + data.size(), '\0');
            const bool got_data = socket->Receive(data, &address);
            if(got_data)
            {
                NetworkMessage message;
                message.id = 77;
                message.payload = data;
                dispatcher->PushNewMessage(message);
            }
            else
            {
                System::Sleep(4);
            }
        }
    };

    const auto send_func = [](
        network::ISocketPtr socket, const network::Address& target, OutgoingMessages* out_messages, const bool& stop) {
        while(!stop)
        {
            bool no_messages = false;

            {
                std::lock_guard<std::mutex> lock(out_messages->message_mutex);
                no_messages = out_messages->unhandled_messages.empty();

                for(const NetworkMessage& message : out_messages->unhandled_messages)
                    socket->SendTo(message.payload, target);

                out_messages->unhandled_messages.clear();
            }

            if(no_messages)
                System::Sleep(4);
        }
    };

    m_incomming_thread = std::thread(comm_func, std::move(in_socket), dispatcher, std::ref(m_stop));
    m_outgoing_thread = std::thread(send_func, std::move(out_socket), target, &m_messages, std::ref(m_stop));
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_incomming_thread.join();
    m_outgoing_thread.join();
}

void RemoteConnection::SendMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_messages.message_mutex);
    m_messages.unhandled_messages.push_back(message);
}
