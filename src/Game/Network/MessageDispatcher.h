
#pragma once

#include "MonoFwd.h"
#include "IUpdatable.h"

#include "NetworkMessage.h"

#include <vector>
#include <unordered_map>
#include <mutex>

namespace network
{
    struct Address;
}

namespace game
{
    class MessageDispatcher : public mono::IUpdatable
    {
    public:

        MessageDispatcher(mono::EventHandler& event_handler);
        void PushNewMessage(const NetworkMessage& message);
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:

        mono::EventHandler& m_event_handler;
        std::mutex m_message_mutex;
        std::vector<NetworkMessage> m_unhandled_messages;

        using MessageFunc = bool(*)(const byte_view& message, const network::Address& sender, mono::EventHandler& event_handler);
        std::unordered_map<uint32_t, MessageFunc> m_handlers;
    };
}
