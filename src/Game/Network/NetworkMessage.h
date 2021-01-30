
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
        network::Address sender;
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

    struct ClientPlayerSpawned
    {
        DECLARE_NETWORK_MESSAGE();
        uint16_t client_entity_id;
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

    struct LevelMetadataMessage
    {
        DECLARE_NETWORK_MESSAGE();
        math::Vector camera_position;
        math::Vector camera_size;
        uint32_t world_file_hash;
        uint32_t background_texture_hash;
    };

    struct TransformMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t timestamp;
        uint16_t entity_id;
        uint16_t parent_transform;
        math::Vector position;
        float rotation;
    };

    struct SpawnMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint32_t timestamp;
        uint16_t entity_id;
        bool spawn;
    };

    struct SpriteMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint16_t entity_id;
        uint32_t filename_hash;
        uint32_t hex_color;
        uint8_t animation_id;
        int8_t layer;
        uint32_t properties;
        float shadow_offset_x;
        float shadow_offset_y;
        float shadow_size;
    };

    struct DamageInfoMessage
    {
        DECLARE_NETWORK_MESSAGE();
        uint16_t entity_id;
        int16_t health;
        int16_t full_health;
        bool is_boss;
        uint32_t damage_timestamp;
    };

    struct RemoteInputMessage
    {
        DECLARE_NETWORK_MESSAGE();
        network::Address sender;
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
            System::Log("\t%u %s %zu\n", message_name::message_type, #message_name, sizeof(message_name));

        System::Log("Network message sizes\n");

        PRINT_NETWORK_MESSAGE_SIZE(ServerBeaconMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ServerQuitMessage);
        PRINT_NETWORK_MESSAGE_SIZE(PingMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ConnectMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ConnectAcceptedMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ClientPlayerSpawned);
        PRINT_NETWORK_MESSAGE_SIZE(DisconnectMessage);
        PRINT_NETWORK_MESSAGE_SIZE(HeartBeatMessage);
        PRINT_NETWORK_MESSAGE_SIZE(TextMessage);
        PRINT_NETWORK_MESSAGE_SIZE(TransformMessage);
        PRINT_NETWORK_MESSAGE_SIZE(SpawnMessage);
        PRINT_NETWORK_MESSAGE_SIZE(SpriteMessage);
        PRINT_NETWORK_MESSAGE_SIZE(DamageInfoMessage);
        PRINT_NETWORK_MESSAGE_SIZE(RemoteInputMessage);
        PRINT_NETWORK_MESSAGE_SIZE(RemoteCameraMessage);
        PRINT_NETWORK_MESSAGE_SIZE(ViewportMessage);
    }
}
