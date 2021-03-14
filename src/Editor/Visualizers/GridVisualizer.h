
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/RenderFwd.h"
#include <memory>

namespace editor
{
    class GridVisualizer : public mono::IDrawable
    {
    public:

        GridVisualizer(const bool& draw_grid);
        virtual void Draw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const bool& m_draw_grid;
        std::unique_ptr<mono::IRenderBuffer> m_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_colors;
    };
}
