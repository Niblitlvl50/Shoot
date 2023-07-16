
#pragma once

#include "Rendering/IDrawable.h"

namespace animator
{
    class InterfaceDrawer : public mono::IDrawable
    {
    public:

        InterfaceDrawer(struct UIContext& context);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        struct UIContext& m_context;
    };
}
