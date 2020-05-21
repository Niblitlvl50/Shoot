
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderPtrFwd.h"
#include <vector>

namespace editor
{
    class GridVisualizer : public mono::IDrawable
    {
    public:

        GridVisualizer();
        virtual void Draw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const std::vector<math::Vector> m_gridVertices;
    };
}
