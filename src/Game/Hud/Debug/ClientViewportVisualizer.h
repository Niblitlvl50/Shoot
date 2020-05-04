
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"

#include <unordered_map>

namespace network
{
    struct Address;
}

namespace game
{
    struct ClientData;

    class ClientViewportVisualizer : public mono::IDrawable
    {
    public:

        ClientViewportVisualizer(const std::unordered_map<network::Address, ClientData>& client_data);

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const std::unordered_map<network::Address, ClientData>& m_client_data;
    };
}
