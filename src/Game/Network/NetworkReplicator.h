
#pragma once

#include "IUpdatable.h"
#include "NetworkMessage.h"

#include <queue>

class IEntityManager;

namespace mono
{
    class TransformSystem;
    class PhysicsSystem;
    class SpriteSystem;
}

namespace game
{
    class INetworkPipe;

    class NetworkReplicator : public mono::IUpdatable
    {
    public:

        NetworkReplicator(
            mono::TransformSystem* transform_system,
            mono::SpriteSystem* sprite_system,
            IEntityManager* entity_manager,
            INetworkPipe* remote_connection,
            uint32_t replication_interval);
        
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        IEntityManager* m_entity_manager;
        INetworkPipe* m_remote_connection;
        uint32_t m_replication_interval;
        uint32_t m_keyframe_index;

        struct TransformData
        {
            int time_to_replicate;
            math::Vector position;
            float rotation;
        } m_transform_data[500];

        struct SpriteData
        {
            int time_to_replicate;
            uint32_t hex_color;
            short animation_id;
            bool vertical_direction;
            bool horizontal_direction;
        } m_sprite_data[500];

        std::queue<NetworkMessage> m_message_queue;
    };

    class ClientReplicator : public mono::IUpdatable
    {
    public:

        ClientReplicator(INetworkPipe* remote_connection);
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:

        INetworkPipe* m_remote_connection;
    };
}
