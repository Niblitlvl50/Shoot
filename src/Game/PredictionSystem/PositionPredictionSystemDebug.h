
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    class PositionPredictionSystem;

    class PredictionSystemDebugDrawer : public mono::IDrawable
    {
    public:

        PredictionSystemDebugDrawer(const PositionPredictionSystem* prediction_system);
        void Draw(mono::IRenderer& renderer) const;
        math::Quad BoundingBox() const;

        const PositionPredictionSystem* m_prediction_system;
    };
}
