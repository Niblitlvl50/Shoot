
#pragma once

#include "Rendering/IDrawable.h"

namespace game
{
    struct LevelMetadata;
}

namespace editor
{
    class GameCameraVisualizer : public mono::IDrawable
    {
    public:

        GameCameraVisualizer(const bool& enabled, const game::LevelMetadata& metadata);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const bool& m_enabled;
        const game::LevelMetadata& m_metadata;
    };
}
