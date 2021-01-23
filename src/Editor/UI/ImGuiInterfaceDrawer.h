
#pragma once

#include "Rendering/IDrawable.h"

namespace editor
{
    struct UIContext;

    class ImGuiInterfaceDrawer : public mono::IDrawable
    {
    public:
        ImGuiInterfaceDrawer(UIContext& context);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;
    
    private:
        UIContext& m_context;
    };
}

