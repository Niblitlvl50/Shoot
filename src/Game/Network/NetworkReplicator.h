
#pragma once

#include "IUpdatable.h"
#include "NetworkMessage.h"

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

        TransformMessage m_transform_messages[500];
        SpriteMessage m_sprite_messages[500];

        uint32_t m_replication_interval;
        uint32_t m_replicate_timer;
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
