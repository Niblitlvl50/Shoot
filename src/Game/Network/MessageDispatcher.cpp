
#include "MessageDispatcher.h"
#include "NetworkMessage.h"
#include "NetworkSerialize.h"
#include "System/System.h"

#include "EventHandler/EventHandler.h"

#include "ScopedTimer.h"

using namespace game;

namespace
{
    template <typename T>
    bool HandleMessage(const byte_view& network_message, const network::Address& sender, mono::EventHandler* event_handler)
    {
        T decoded_message;
        const bool success = DeserializeMessage(network_message, decoded_message);
        if(success)
            event_handler->DispatchEvent(decoded_message);
        return success;
    }

    template <typename T>
    bool HandleMessageAndSender(const byte_view& network_message, const network::Address& sender, mono::EventHandler* event_handler)
    {
        T decoded_message;
        const bool success = DeserializeMessage(network_message, decoded_message);
        decoded_message.sender = sender;
        if(success)
            event_handler->DispatchEvent(decoded_message);
        return success;
    }
}

#define REGISTER_MESSAGE_HANDLER(message) \
    m_handlers[message::message_type] = HandleMessage<message>;

#define REGISTER_MESSAGE_HANDLER_WITH_SENDER(message) \
    m_handlers[message::message_type] = HandleMessageAndSender<message>;

MessageDispatcher::MessageDispatcher(mono::EventHandler* event_handler)
    : m_event_handler(event_handler)
    , m_push_messages(&m_message_buffer_1)
{
    REGISTER_MESSAGE_HANDLER(ServerQuitMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(ServerBeaconMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(PingMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(ConnectMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(ConnectAcceptedMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(DisconnectMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(HeartBeatMessage);

    REGISTER_MESSAGE_HANDLER(TextMessage);
    REGISTER_MESSAGE_HANDLER(TransformMessage);
    REGISTER_MESSAGE_HANDLER(SpawnMessage);
    REGISTER_MESSAGE_HANDLER(SpriteMessage);
    REGISTER_MESSAGE_HANDLER(RemoteInputMessage);
    REGISTER_MESSAGE_HANDLER(RemoteCameraMessage);
    REGISTER_MESSAGE_HANDLER_WITH_SENDER(ViewportMessage);
}

void MessageDispatcher::PushNewMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);
    m_push_messages->push_back(message);
}

void MessageDispatcher::doUpdate(const mono::UpdateContext& update_context)
{
    std::vector<NetworkMessage>* unhandled_messages;

    {
        std::lock_guard<std::mutex> lock(m_message_mutex);
        unhandled_messages = m_push_messages;
        m_push_messages = (m_push_messages == &m_message_buffer_1) ? &m_message_buffer_2 : &m_message_buffer_1;
    }

    for(const NetworkMessage& network_message : *unhandled_messages)
    {
        const std::vector<byte_view>& n_messages = UnpackMessageBuffer(network_message.payload);
        for(size_t index = 0; index < n_messages.size(); ++index)
        {
            const byte_view& message_view = n_messages[index];
            const uint32_t message_type = PeekMessageType(message_view);

            const auto handler_it = m_handlers.find(message_type);
            if(handler_it == m_handlers.end())
            {
                System::Log(
                    "network|Failed to find a handler for message of type: %u, message: %lu/%lu\n", message_type, index, n_messages.size());
                continue;
            }

            const bool handled_message = handler_it->second(message_view, network_message.address, m_event_handler);
            if(!handled_message)
                System::Log("network|Failed to deserialize message of type: %u\n", message_type);
        }
    }

    unhandled_messages->clear();
}
