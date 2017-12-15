
#pragma once

#include <vector>

using byte = unsigned char;

namespace game
{
    struct NetworkMessage
    {
        unsigned int id;
        std::vector<byte> payload;
    };

    struct TextMessage
    {
        char text[256];
    };

    struct PositionalMessage
    {
        unsigned int entity_id;
        float x;
        float y;
        float rotation;
    };

    struct SpawnMessage
    {
        unsigned int spawn_type_id;
        unsigned int assigned_id;
    };

    template <typename T>
    std::vector<byte> SerializeMessage(const T& message)
    {
        const size_t type_hash = typeid(T).hash_code();
        constexpr size_t id_size = sizeof(size_t);
        constexpr size_t message_size = sizeof(T);

        std::vector<byte> payload;
        payload.resize(id_size + message_size);

        std::memcpy(payload.data(), &type_hash, id_size);
        std::memcpy(payload.data() + id_size, &message, message_size);

        return payload;
    }
}
