
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"

namespace editor
{
    class IObjectProxy;

    class ObjectDetailVisualizer : public mono::IDrawable
    {
    public:

        ObjectDetailVisualizer();
        void SetObjectProxy(IObjectProxy* object_proxy);

        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        IObjectProxy* m_object_proxy = nullptr;
    };
}
