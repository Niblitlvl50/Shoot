
#include "MessageDispatcher.h"
#include "NetworkMessage.h"

using namespace game;

void MessageDispatcher::PushNewMessage(const NetworkMessage& message)
{
    std::lock_guard<std::mutex> lock(m_message_mutex);
    m_unhandled_messages.push_back(message);
}

void MessageDispatcher::RegisterHandler(const std::string& message_name)
{
}

void MessageDispatcher::doUpdate(unsigned int delta)
{
    //constexpr size_t afsd = typeid(TextMessage).hash_code();

    std::lock_guard<std::mutex> lock(m_message_mutex);
    for(const NetworkMessage& message : m_unhandled_messages)
    {
        // Do stuff here
        std::printf("Network message with id: %u\n", message.id);
    }

    m_unhandled_messages.clear();
}
