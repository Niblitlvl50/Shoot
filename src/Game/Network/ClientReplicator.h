
#pragma once

#include "IUpdatable.h"
#include "NetworkMessage.h"
#include "NetworkSerialize.h"

namespace mono
{
    class ICamera;
}

namespace game
{
    class ClientManager;

    class ClientReplicator : public mono::IUpdatable
    {
    public:

        ClientReplicator(mono::ICamera* camera, ClientManager* remote_connection);
        void Update(const mono::UpdateContext& update_context) override;

    private:

        mono::ICamera* m_camera;
        ClientManager* m_remote_connection;
        uint32_t m_replicate_timer;
    };
}
