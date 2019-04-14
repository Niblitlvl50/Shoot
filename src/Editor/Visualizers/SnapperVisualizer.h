
#pragma once

#include "Rendering/IDrawable.h"
#include <vector>

namespace editor
{
    struct SnapPoint;

    class SnapperVisualizer : public mono::IDrawable
    {
    public:

        SnapperVisualizer(const bool& draw_snappers, const std::vector<SnapPoint>& snappers);
        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const bool& m_draw_snappers;
        const std::vector<SnapPoint>& m_snappers;
    };
}
