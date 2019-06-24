
#include "MessageDispatcher.h"
#include "NetworkMessage.h"

#include "EventHandler/EventHandler.h"

#include "ScopedTimer.h"

using namespace game;

namespace
{
    template <typename T>
    bool HandleMessage(const NetworkMessage& network_message, mono::EventHandler& event_handler)
    {
        T decoded_message;
        const bool success = DeserializeMessage(network_message, decoded_message);
        if(success)
            event_handler.DispatchEvent(decoded_message);
        return success;
    }
}

#define REGISTER_MESSAGE_HANDLER(message) \
    m_handlers[message::message_type] = HandleMessage<message>;

MessageDispatcher::MessageDispatcher(mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    REGISTER_MESSAGE_HANDLER(ServerBeaconMessage);
    REGISTER_MESSAGE_HANDLER(ServerQuitMessage);
    REGISTER_MESSAGE_HANDLER(PingMessage);
    REGISTER_MESSAGE_HANDLER(ConnectMessage);
    REGISTER_MESSAGE_HANDLER(ConnectAcceptedMessage);
    REGISTER_MESSAGE_HANDLER(DisconnectMessage);
    REGISTER_MESSAGE_HANDLER(HeartBeatMessage);

    REGISTER_MESSAGE_HANDLER(TextMessage);
    REGISTER_MESSAGE_HANDLER(TransformMessage);
    REGISTER_MESSAGE_HANDLER(SpawnMessage);
    REGISTER_MESSAGE_HANDLER(SpriteMessage);
    REGISTER_MESSAGE_HANDLER(RemoteInputMessage);
}

void MessageDispatcher::PushNewMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);
    m_unhandled_messages.push_back(message);
}

void MessageDispatcher::doUpdate(const mono::UpdateContext& update_context)
{
    //SCOPED_TIMER_AUTO();

    std::lock_guard<std::mutex> lock(m_message_mutex);

    for(const NetworkMessage& network_message : m_unhandled_messages)
    {
        const uint32_t message_type = PeekMessageType(network_message.payload);

        auto handler_it = m_handlers.find(message_type);
        if(handler_it == m_handlers.end())
        {
            std::printf("network|Failed to find a message for message type: %u\n", message_type);
            continue;
        }

        const bool handled_message = handler_it->second(network_message, m_event_handler);
        if(!handled_message)
            std::printf("network|Failed to deserialize network message with id %u\n", message_type);
    }

    m_unhandled_messages.clear();
}
