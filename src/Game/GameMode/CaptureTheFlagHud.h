
#pragma once

#include "Rendering/IDrawable.h"
#include "CaptureTheFlagScore.h"

namespace game
{
    class CaptureTheFlagHud : public mono::IDrawable
    {
    public:

        CaptureTheFlagHud(const CaptureTheFlagScore& score);

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const CaptureTheFlagScore& m_score;
    };
}
