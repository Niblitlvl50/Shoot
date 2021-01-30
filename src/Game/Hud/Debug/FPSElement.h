
#pragma once

#include "Rendering/IDrawable.h"
#include "Util/FpsCounter.h"

namespace game
{
    class FPSElement : public mono::IDrawable
    {
    public:

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;
        mutable mono::FpsCounter m_counter;
    };
}
