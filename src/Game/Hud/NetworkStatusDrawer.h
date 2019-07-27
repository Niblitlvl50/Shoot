
#pragma once

#include "Entity/EntityBase.h"

namespace game
{
    class NetworkStatusDrawer : public mono::EntityBase
    {
    public:

        NetworkStatusDrawer(const math::Vector& position, const class INetworkPipe* network_pipe);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

        const INetworkPipe* m_network_pipe;
        uint32_t m_total_frame_count;
    };
}
