
#pragma once

#include "IUpdatable.h"

#include <vector>
#include <string>
#include <mutex>

namespace game
{
    struct NetworkMessage;

    class MessageDispatcher : public mono::IUpdatable
    {
    public:

        void PushNewMessage(const NetworkMessage& message);
        void RegisterHandler(const std::string& message_name);

    private:

        void doUpdate(unsigned int delta);

        std::vector<NetworkMessage> m_unhandled_messages;
        std::mutex m_message_mutex;
    };
}
