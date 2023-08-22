
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    class TargetSystemDrawer : public mono::IDrawable
    {
    public:

        TargetSystemDrawer(const bool& enable_drawing, class TargetSystem* target_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:
        const bool& m_enable_drawing;
        class TargetSystem* m_target_system;
    };
}
