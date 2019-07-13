
#pragma once

#include "Math/Vector.h"
#include "System/Network.h"
#include "System/System.h"

#include <vector>
#include <cstdint>
#include <string_view>


using byte = unsigned char;
using byte_view = std::basic_string_view<byte>;

#define DECLARE_NETWORK_MESSAGE() \
    static constexpr uint32_t message_type  = __COUNTER__; \
    NetworkMessageHeader header;

namespace game
{
    constexpr uint32_t NetworkMessageBufferSize = 1024;

    struct NetworkMessageHeader
    {
        network::Address sender;
    };

    struct NetworkMessage
    {
        network::Address address;
        std::vector<byte> payload;
    };

    struct ServerBeaconMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct ServerQuitMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct PingMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t local_time_stamp;
    };

    struct ConnectMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct ConnectAcceptedMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct DisconnectMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct HeartBeatMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct TextMessage
    {
        DECLARE_NETWORK_MESSAGE();
        char text[256] = { 0 };
    };

    struct TransformMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t entity_id;
        math::Vector position;
        float rotation;
    };

    struct SpawnMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t entity_id;
        bool spawn;
    };

    struct SpriteMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t entity_id;
        uint32_t filename_hash;
        uint32_t hex_color;
        short animation_id;
        bool vertical_direction;
        bool horizontal_direction; 
    };

    struct RemoteInputMessage
    {
        DECLARE_NETWORK_MESSAGE();
        System::ControllerState controller_state;
    };

    template <typename T>
    inline bool DeserializeMessage(const byte_view& message, T& deserialized_message)
    {
        constexpr size_t message_type_size = sizeof(T::message_type);
        constexpr size_t message_size = sizeof(T);
        constexpr size_t type_and_message_size = message_type_size + message_size;

        if(message.size() != type_and_message_size)
        {
            std::printf("Payload size missmatch! %zu / %zu\n", message.size(), type_and_message_size);
            return false;
        }

        uint32_t message_type = 0;
        std::memcpy(&message_type, message.data(), message_type_size);

        if(T::message_type != message_type)
        {
            std::printf("Message id missmatch!\n");
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

    inline void PrintNetworkMessageSize()
    {
        #define PRINT_NETWORK_MESSAGE_SIZE(message_name) \
            std::printf("\t%s %zu\n", #message_name, sizeof(message_name));

        std::printf("Network message sizes\n");

        PRINT_NETWORK_MESSAGE_SIZE(NetworkMessageHeader);
        PRINT_NETWORK_MESSAGE_SIZE(ServerBeaconMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ServerQuitMessage);
        PRINT_NETWORK_MESSAGE_SIZE(PingMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ConnectMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ConnectAcceptedMessage);
        PRINT_NETWORK_MESSAGE_SIZE(DisconnectMessage);
        PRINT_NETWORK_MESSAGE_SIZE(HeartBeatMessage);
        PRINT_NETWORK_MESSAGE_SIZE(TextMessage);
        PRINT_NETWORK_MESSAGE_SIZE(TransformMessage);
        PRINT_NETWORK_MESSAGE_SIZE(SpawnMessage);
        PRINT_NETWORK_MESSAGE_SIZE(SpriteMessage);
        PRINT_NETWORK_MESSAGE_SIZE(RemoteInputMessage);
    }

    inline void PrepareMessageBuffer(std::vector<byte>& message_buffer)
    {
        message_buffer.reserve(NetworkMessageBufferSize);
        constexpr size_t buffer_header = sizeof(uint32_t) * 2;

        // Make room for two uint32_t, one for id and the second one for number of messages in buffer.
        message_buffer.resize(buffer_header, '\0');
    }

    inline void GetMessageBufferHeader(const std::vector<byte>& message_buffer, uint32_t& out_id, uint32_t& out_n_messages)
    {
        out_id = *message_buffer.data();
        out_n_messages = *(message_buffer.data() + sizeof(uint32_t));
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

        // Increment number of messages in message buffer (this could be done in place)
        {
            uint32_t n_messages = *(message_buffer.data() + sizeof(uint32_t));
            n_messages++;
            std::memcpy(message_buffer.data() + sizeof(uint32_t), &n_messages, sizeof(uint32_t));
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
        const uint32_t n_messages = *(message_buffer.data() + sizeof(uint32_t));

        std::vector<byte_view> buffer_views;
        buffer_views.reserve(n_messages);

        constexpr size_t header_offset = sizeof(uint32_t) * 2;
        constexpr size_t payload_data_size = sizeof(uint32_t);

        size_t previous_position = header_offset;

        for(uint32_t index = 0; index < n_messages; ++index)
        {
            uint32_t payload_length = 0;
            std::memcpy(&payload_length, message_buffer.data() + previous_position, payload_data_size);
            buffer_views.emplace_back(message_buffer.data() + previous_position + payload_data_size, payload_length);
            previous_position += payload_data_size + payload_length;
        }

        return buffer_views;
    }
}
