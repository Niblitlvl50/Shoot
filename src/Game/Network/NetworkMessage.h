
#pragma once

#include <vector>
#include <stdlib.h>
#include <cstring>

using byte = unsigned char;

namespace game
{
    struct NetworkMessage
    {
        uint32_t id;
        std::vector<byte> payload;
    };

    struct TextMessage
    {
        static constexpr uint32_t message_id = 0;
        char text[256] = { 0 };
    };

    struct PositionalMessage
    {
        static constexpr uint32_t message_id = 1;

        uint32_t entity_id;
        float x;
        float y;
        float rotation;
    };

    struct SpawnMessage
    {
        static constexpr uint32_t message_id = 2;

        uint32_t spawn_type_id;
        uint32_t assigned_entity_id;
        float x;
        float y;
    };

    struct DespawnMessage
    {
        static constexpr uint32_t message_id = 3;
        uint32_t entity_id;
    };

    struct AnimationMessage
    {
        static constexpr uint32_t message_id = 4;
        uint32_t entity_id;
        int animation_id;
    };

    template <typename T>
    inline std::vector<byte> SerializeMessage(const T& message)
    {
        constexpr size_t type_hash = T::message_id;

        constexpr size_t id_type_size = sizeof(size_t);
        constexpr size_t payload_type_size = sizeof(size_t);
        constexpr size_t message_size = sizeof(T);
        
        std::vector<byte> payload;
        payload.resize(id_type_size + payload_type_size + message_size);
        std::memcpy(payload.data(), &type_hash, id_type_size);
        std::memcpy(payload.data() + id_type_size, &message_size, payload_type_size);
        std::memcpy(payload.data() + id_type_size + payload_type_size, &message, message_size);

        return payload;
    }

    template <typename T>
    inline bool DeserializeMessage(const std::vector<byte>& message, T& deserialized_message)
    {
        constexpr size_t id_type_size = sizeof(T::message_id);
        constexpr size_t payload_type_size = sizeof(size_t);
        constexpr size_t message_size = sizeof(T);

        uint32_t message_id = 0;
        std::memcpy(&message_id, message.data(), id_type_size);

        if(T::message_id != message_id)
            return false;

        size_t payload_size = 0;
        std::memcpy(&payload_size, message.data() + id_type_size, payload_type_size);

        if(payload_size != message_size)
            return false;

        std::memcpy(&deserialized_message, message.data() + id_type_size + payload_type_size, message_size);
        return true;
    }

    inline uint32_t PeekMessageType(const std::vector<byte>& message)
    {
        constexpr size_t id_type_size = sizeof(uint32_t);

        uint32_t message_id = 0;
        std::memcpy(&message_id, message.data(), id_type_size);
    
        return message_id;
    }
}
