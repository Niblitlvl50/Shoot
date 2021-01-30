
#pragma once

#include "Rendering/IDrawable.h"
#include "IUpdatable.h"

namespace game
{
    class NetworkStatusDrawer : public mono::IDrawable
    {
    public:

        NetworkStatusDrawer(const class INetworkPipe* network_pipe);
        //void EntityDraw(mono::IRenderer& renderer) const override;
        //void Update(const mono::UpdateContext& update_context) override;

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const INetworkPipe* m_network_pipe;
        uint32_t m_total_frame_count;
    };
}
