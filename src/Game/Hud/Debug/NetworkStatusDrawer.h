
#pragma once

#include "Zone/EntityBase.h"

namespace game
{
    class NetworkStatusDrawer : public mono::EntityBase
    {
    public:

        NetworkStatusDrawer(const math::Vector& position, const class INetworkPipe* network_pipe);
        void EntityDraw(mono::IRenderer& renderer) const override;
        void EntityUpdate(const mono::UpdateContext& update_context) override;

        const INetworkPipe* m_network_pipe;
        uint32_t m_total_frame_count;
    };
}
