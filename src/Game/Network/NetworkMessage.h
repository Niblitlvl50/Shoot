
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "System/Network.h"
#include "System/System.h"

#include <cstdint>


#define DECLARE_NETWORK_MESSAGE() \
    static constexpr uint32_t message_type  = __COUNTER__; \

namespace game
{
    struct ServerBeaconMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address server_address;
    };

    struct ServerQuitMessage
    {
        DECLARE_NETWORK_MESSAGE();
    };

    struct PingMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t server_time;
        uint32_t local_time;
        network::Address sender;
    };

    struct ConnectMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
    };

    struct ConnectAcceptedMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
    };

    struct DisconnectMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
    };

    struct HeartBeatMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
    };

    struct TextMessage
    {
        DECLARE_NETWORK_MESSAGE();
        char text[256] = { 0 };
    };

    struct TransformMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t timestamp;
        uint32_t entity_id;
        math::Vector position;
        float rotation;
        bool settled;
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
        uint32_t id;
        System::ControllerState controller_state;
    };

    struct RemoteCameraMessage
    {
        DECLARE_NETWORK_MESSAGE();
        math::Vector position;
        math::Quad viewport;
    };

    struct ViewportMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
        math::Quad viewport;
    };

    inline void PrintNetworkMessageSize()
    {
        #define PRINT_NETWORK_MESSAGE_SIZE(message_name) \
            std::printf("\t%u %s %zu\n", message_name::message_type, #message_name, sizeof(message_name));

        std::printf("Network message sizes\n");

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
        PRINT_NETWORK_MESSAGE_SIZE(RemoteCameraMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ViewportMessage);
    }
}
