
#pragma once

#include "INetworkPipe.h"
#include "NetworkMessage.h"

namespace game
{
    class BatchedMessageSender
    {
    public:

        BatchedMessageSender(game::INetworkPipe* network_pipe)
            : m_network_pipe(network_pipe)
        {
            PrepareMessageBuffer(m_out_message.payload);
        }

        ~BatchedMessageSender()
        {
            if(!m_out_message.payload.empty())
                m_network_pipe->SendMessage(m_out_message);
        }

        template <typename T>
        void SendMessage(const T& message)
        {
            const bool success = SerializeMessageToBuffer(message, m_out_message.payload);
            if(!success)
            {
                m_network_pipe->SendMessage(m_out_message);
                m_out_message.payload.clear();
                PrepareMessageBuffer(m_out_message.payload);
            
                SerializeMessageToBuffer(message, m_out_message.payload);
            }
        }

    private:

        game::INetworkPipe* m_network_pipe;
        NetworkMessage m_out_message;
    };
}
