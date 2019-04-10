
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"

#include <vector>
#include <unordered_map>
#include <mutex>

namespace game
{
    struct NetworkMessage;

    class MessageDispatcher : public mono::IUpdatable
    {
    public:

        MessageDispatcher(mono::EventHandler& event_handler);
        void PushNewMessage(const NetworkMessage& message);

    private:

        void doUpdate(unsigned int delta);

        mono::EventHandler& m_event_handler;
        std::mutex m_message_mutex;
        std::vector<NetworkMessage> m_unhandled_messages;

        using MessageFunc = bool(*)(const NetworkMessage& message, mono::EventHandler& event_handler);
        std::unordered_map<uint32_t, MessageFunc> m_handlers;
    };
}
