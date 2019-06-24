
#pragma once

#include "Math/Matrix.h"
#include "Rendering/Color.h"
#include "System/Network.h"
#include "System/System.h"

#include <vector>
#include <stdlib.h>
#include <cstring>
#include <cstdint>
#include <cstdio>


using byte = unsigned char;

#define DECLARE_NETWORK_MESSAGE() \
    static constexpr uint32_t message_type  = __COUNTER__; \
    uint32_t message_id; \
    network::Address sender_address; 

namespace game
{
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
        math::Matrix transform;
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
        mono::Color::RGBA shade;
        int vertical_direction;
        int horizontal_direction; 
        int animation_id;
    };

    struct RemoteInputMessage
    {
        DECLARE_NETWORK_MESSAGE();
        System::ControllerState controller_state;
    };

    template <typename T>
    inline std::vector<byte> SerializeMessage(const T& message)
    {
        constexpr uint32_t type_hash = T::message_type;

        constexpr size_t id_type_size = sizeof(uint32_t);
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
    inline bool DeserializeMessage(const NetworkMessage& message, T& deserialized_message)
    {
        constexpr size_t message_type_size = sizeof(T::message_type);
        constexpr size_t payload_type_size = sizeof(size_t);
        constexpr size_t message_size = sizeof(T);

        const std::vector<byte>& payload = message.payload;

        uint32_t message_type = 0;
        std::memcpy(&message_type, payload.data(), message_type_size);

        if(T::message_type != message_type)
        {
            std::printf("Message id missmatch!\n");
            return false;
        }

        size_t payload_size = 0;
        std::memcpy(&payload_size, payload.data() + message_type_size, payload_type_size);

        if(payload_size != message_size)
        {
            std::printf("Payload size missmatch! %zu / %zu\n", payload_size, message_size);
            return false;
        }

        std::memcpy(&deserialized_message, payload.data() + message_type_size + payload_type_size, message_size);
        deserialized_message.sender_address = message.address;
        return true;
    }

    inline uint32_t PeekMessageType(const std::vector<byte>& message)
    {
        constexpr size_t message_type_size = sizeof(uint32_t);

        uint32_t message_type = 0;
        std::memcpy(&message_type, message.data(), message_type_size);
    
        return message_type;
    }
}
