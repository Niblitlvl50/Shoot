
#pragma once

#include "Rendering/IDrawable.h"

namespace editor
{
    class GameCameraVisualizer : public mono::IDrawable
    {
    public:

        GameCameraVisualizer(const math::Vector& position, const math::Vector& size);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const math::Vector& m_position;
        const math::Vector& m_size;
    };
}
