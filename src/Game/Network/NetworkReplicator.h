
#pragma once

#include "IUpdatable.h"

class IEntityManager;

namespace mono
{
    class TransformSystem;
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
            INetworkPipe* remote_connection);
        
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        IEntityManager* m_entity_manager;
        INetworkPipe* m_remote_connection;
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
