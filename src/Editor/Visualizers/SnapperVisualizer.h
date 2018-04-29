
#pragma once

#include "Rendering/IDrawable.h"
#include <vector>

namespace editor
{
    struct SnapPoint;

    class SnapperVisualizer : public mono::IDrawable
    {
    public:

        SnapperVisualizer(const std::vector<editor::SnapPoint>& snappers);
        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const std::vector<editor::SnapPoint>& m_snappers;
    };

}
