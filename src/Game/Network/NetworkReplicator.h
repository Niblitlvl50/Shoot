
#pragma once

#include "IUpdatable.h"

namespace mono
{
    class PhysicsZone;
    class SpriteSystem;
}

namespace game
{
    class RemoteConnection;

    class NetworkReplicator : public mono::IUpdatable
    {
    public:

        NetworkReplicator(
            const mono::PhysicsZone* physics_zone, mono::SpriteSystem* sprites, RemoteConnection* remote_connection);
        void doUpdate(unsigned int delta) override;

    private:

        const mono::PhysicsZone* m_physics_zone;
        mono::SpriteSystem* m_sprite_system;
        RemoteConnection* m_remote_connection;
    };
}
