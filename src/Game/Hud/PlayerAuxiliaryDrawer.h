
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"

#include <memory>

namespace game
{
    class PlayerAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;

        std::unique_ptr<mono::IRenderBuffer> m_laser_vertices;
        std::unique_ptr<mono::IRenderBuffer> m_laser_colors;
        std::unique_ptr<mono::IElementBuffer> m_laser_indices;
    };
}
