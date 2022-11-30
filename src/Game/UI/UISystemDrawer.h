
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class UISystem;

    class UISystemDrawer : public mono::IDrawable
    {
    public:

        UISystemDrawer(const UISystem* ui_system, mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const UISystem* m_ui_system;
        const mono::TransformSystem* m_transform_system;
    };
}
