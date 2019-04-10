
#include "MessageDispatcher.h"
#include "NetworkMessage.h"

#include "EventHandler/EventHandler.h"

using namespace game;

namespace
{
    template <typename T>
    bool HandleMessage(const NetworkMessage& network_message, mono::EventHandler& event_handler)
    {
        T decoded_message;
        const bool success = DeserializeMessage(network_message.payload, decoded_message);
        if(success)
            event_handler.DispatchEvent(decoded_message);
        return success;
    }
}

MessageDispatcher::MessageDispatcher(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    m_handlers[TextMessage::message_id]         = HandleMessage<TextMessage>;
    m_handlers[PositionalMessage::message_id]   = HandleMessage<PositionalMessage>;
    m_handlers[SpawnMessage::message_id]        = HandleMessage<SpawnMessage>;
    m_handlers[DespawnMessage::message_id]      = HandleMessage<DespawnMessage>;
    m_handlers[AnimationMessage::message_id]    = HandleMessage<AnimationMessage>;
}

void MessageDispatcher::PushNewMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);
    m_unhandled_messages.push_back(message);
}

void MessageDispatcher::doUpdate(unsigned int delta)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);

    for(const NetworkMessage& network_message : m_unhandled_messages)
    {
        const uint32_t message_type = PeekMessageType(network_message.payload);
        const bool handled_message = m_handlers[message_type](network_message, m_event_handler);

        if(!handled_message)
            std::printf("network|Failed to deserialize network message.\n");
    }

    m_unhandled_messages.clear();
}
