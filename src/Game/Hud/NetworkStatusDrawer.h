
#pragma once

#include "Entity/EntityBase.h"

namespace game
{
    class ServerStatusDrawer : public mono::EntityBase
    {
    public:

        ServerStatusDrawer(const math::Vector& position, const class ServerManager* server_manager);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

        const ServerManager* m_server_manager;
        uint32_t m_total_frame_count;
    };

    class ClientStatusDrawer : public mono::EntityBase
    {
    public:

        ClientStatusDrawer(const math::Vector& position, const class ClientManager* client_manager);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

        const ClientManager* m_client_manager;
        uint32_t m_total_frame_count;
    };
}
