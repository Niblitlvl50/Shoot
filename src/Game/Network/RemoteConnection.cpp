
#include "RemoteConnection.h"
#include "MessageDispatcher.h"

#include "System/Network.h"
#include "System/System.h"

using namespace game;

RemoteConnection::RemoteConnection(MessageDispatcher* dispatcher)
    : m_stop(false)
{
    const auto comm_func = [](MessageDispatcher* dispatcher, const bool& stop) {
        Network::ISocketPtr comm_socket = Network::CreateUDPSocket(16666, false); 
        std::vector<byte> data(1024, '\0');

        while(!stop)
        {
            const bool got_data = comm_socket->Receive(data);
            if(got_data)
            {
                std::printf("incomming data: %s\n", data.data());
                std::fill(data.begin(), data.begin() + data.size(), '\0');

                NetworkMessage message;
                message.id = 77;
                dispatcher->PushNewMessage(message);
            }
            else
            {
                System::Sleep(10);
            }
        }
    };

    const auto out_func = [](const bool& stop, std::vector<NetworkMessage>* messages) {
        
        Network::ISocketPtr comm_socket = Network::CreateUDPSocket(16666, false);

        while(!stop)
        {
            //std::lock_guard<std::mutex> lock(m_message_mutex);

            for(const NetworkMessage& message : *messages)
            {
                (void)message;
            }

            System::Sleep(10);
        }
    };

    (void)out_func;

    m_comm_thread = std::thread(comm_func, dispatcher, std::ref(m_stop));
    //m_outgoing_thread = std::thread(out_func, m_stop, &m_unhandled_messages);
}

RemoteConnection::~RemoteConnection()
{
    m_stop = true;
    m_comm_thread.join();
    //m_outgoing_thread.join();
}

void RemoteConnection::SendMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);
    m_unhandled_messages.push_back(message);
}
