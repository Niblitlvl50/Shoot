
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"
#include "ObjectProxies/IObjectProxy.h"

#include <vector>

namespace editor
{
    class ObjectNameVisualizer : public mono::IDrawable
    {
    public:

        ObjectNameVisualizer(const bool& enabled, const std::vector<IObjectProxyPtr>& object_proxies);

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const bool& m_enabled;
        const std::vector<IObjectProxyPtr>& m_object_proxies;
    };
}
