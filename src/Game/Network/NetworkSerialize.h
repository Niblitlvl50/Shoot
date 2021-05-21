
#pragma once

#include "System/Network.h"
#include "System/System.h"

#include <vector>
#include <cstdint>
#include <string_view>
#include <cstring>

using byte = uint8_t;
using byte_view = std::basic_string_view<byte>;

namespace game
{
    struct NetworkMessage
    {
        network::Address address;
        std::vector<byte> payload;
    };

    struct NetworkMessageHeader
    {
        uint32_t id;
        uint32_t n_messages;
        uint16_t payload_length;
        uint8_t compressed_payload;
        //uint8_t padding;
    };

    constexpr uint32_t NetworkMessageBufferTotalSize = 1024;
    constexpr uint32_t NetworkMessageBufferSize = NetworkMessageBufferTotalSize - sizeof(NetworkMessageHeader);
    
    struct NewNetworkMessage
    {
        NetworkMessageHeader header;
        byte payload[NetworkMessageBufferSize];
    };

    static_assert(sizeof(game::NewNetworkMessage) == game::NetworkMessageBufferTotalSize);
    
    template <typename T>
    inline bool DeserializeMessage(const byte_view& message, T& deserialized_message)
    {
        constexpr size_t message_type_size = sizeof(T::message_type);
        constexpr size_t message_size = sizeof(T);
        constexpr size_t type_and_message_size = message_type_size + message_size;

        if(message.size() != type_and_message_size)
        {
            System::Log("NetworkSerialize|Payload size missmatch! %zu / %zu", message.size(), type_and_message_size);
            return false;
        }

        uint32_t message_type = 0;
        std::memcpy(&message_type, message.data(), message_type_size);

        if(T::message_type != message_type)
        {
            System::Log("NetworkSerialize|Message id missmatch!");
            return false;
        }

        std::memcpy(&deserialized_message, message.data() + message_type_size, message_size);
        return true;
    }

    inline uint32_t PeekMessageType(const byte_view& message)
    {
        uint32_t message_type = 0;
        std::memcpy(&message_type, message.data(), sizeof(uint32_t));
        return message_type;
    }

    inline void PrepareMessageBuffer(std::vector<byte>& message_buffer)
    {
        message_buffer.reserve(NetworkMessageBufferTotalSize);
        // Allocate space for the message header
        message_buffer.resize(sizeof(NetworkMessageHeader), '\0');
    }

    inline void SetMessageBufferHeader(std::vector<byte>& message_buffer, const NetworkMessageHeader& header)
    {
        std::memcpy(message_buffer.data(), &header, sizeof(NetworkMessageHeader));
    }

    inline NetworkMessageHeader GetMessageBufferHeader(const std::vector<byte>& message_buffer)
    {
        NetworkMessageHeader header;
        std::memcpy(&header, message_buffer.data(), sizeof(NetworkMessageHeader));
        return header;
    }

    template <typename T>
    inline bool SerializeMessageToBuffer(const T& message, std::vector<byte>& message_buffer)
    {
        constexpr size_t payload_type_size = sizeof(uint32_t);
        constexpr size_t type_hash_size = sizeof(T::message_type);
        constexpr size_t message_size = sizeof(T);
        
        constexpr size_t total_size_needed = payload_type_size + type_hash_size + message_size;
        const size_t avalible_space = message_buffer.capacity() - message_buffer.size();

        if(avalible_space < total_size_needed)
            return false;

        {
            // Increment number of messages in message buffer
            NetworkMessageHeader header = GetMessageBufferHeader(message_buffer);
            ++header.n_messages;
            SetMessageBufferHeader(message_buffer, header);
        }

        const size_t current_size = message_buffer.size();
        message_buffer.resize(current_size + total_size_needed, '\0');

        const uint32_t type_and_message_size = type_hash_size + message_size;

        const uint32_t payload_size_offset  = current_size;
        const uint32_t type_hash_offset     = current_size + payload_type_size;
        const uint32_t message_offset       = current_size + payload_type_size + type_hash_size;

        std::memcpy(message_buffer.data() + payload_size_offset,    &type_and_message_size, payload_type_size);
        std::memcpy(message_buffer.data() + type_hash_offset,       &T::message_type,       type_hash_size);
        std::memcpy(message_buffer.data() + message_offset,         &message,               message_size);

        return true;
    }

    template <typename T>
    inline std::vector<byte> SerializeMessage(const T& message)
    {
        std::vector<byte> message_buffer;
        PrepareMessageBuffer(message_buffer);
        SerializeMessageToBuffer(message, message_buffer);
        return message_buffer;
    }

    inline std::vector<byte_view> UnpackMessageBuffer(const std::vector<byte>& message_buffer)
    {
        const NetworkMessageHeader header = GetMessageBufferHeader(message_buffer);

        std::vector<byte_view> buffer_views;
        buffer_views.reserve(header.n_messages);

        constexpr size_t header_offset = sizeof(NetworkMessageHeader);
        constexpr size_t payload_data_size = sizeof(uint32_t);

        size_t previous_position = header_offset;

        for(uint32_t index = 0; index < header.n_messages; ++index)
        {
            uint32_t payload_length = 0;
            std::memcpy(&payload_length, message_buffer.data() + previous_position, payload_data_size);
            buffer_views.emplace_back(message_buffer.data() + previous_position + payload_data_size, payload_length);
            previous_position += (payload_data_size + payload_length);
        }

        return buffer_views;
    }
}
