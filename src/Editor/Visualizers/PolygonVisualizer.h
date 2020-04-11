
#pragma once

#include "Rendering/IDrawable.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Vector.h"
#include <vector>

namespace editor
{
    class PolygonVisualizer2 : public mono::IDrawable
    {
    public:

        PolygonVisualizer2(const std::vector<math::Vector>& points, const math::Vector& mouse_position);
        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

    private:

        const std::vector<math::Vector>& m_points;
        const math::Vector& m_mousePosition;
        mono::ITexturePtr m_texture;
    };
}
