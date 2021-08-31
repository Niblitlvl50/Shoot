
#pragma once

#include "Rendering/IDrawable.h"

namespace shared
{
    struct LevelMetadata;
}

namespace editor
{
    class GameCameraVisualizer : public mono::IDrawable
    {
    public:

        GameCameraVisualizer(const bool& enabled, const shared::LevelMetadata& metadata);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const bool& m_enabled;
        const shared::LevelMetadata& m_metadata;
    };
}
