
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    class UISystemDrawer : public mono::IDrawable
    {
    public:

        UISystemDrawer(const class UISystem* ui_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const class UISystem* m_ui_system;
    };
}
