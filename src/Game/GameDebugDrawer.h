
#pragma once

#include "IDebugDrawer.h"
#include "Rendering/IDrawable.h"

#include "Math/Vector.h"
#include "Rendering/Color.h"

#include <vector>

namespace game
{
    class GameDebugDrawer : public IDebugDrawer, public mono::IDrawable
    {
    public:

        GameDebugDrawer();
        ~GameDebugDrawer();

        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color) override;
        void DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color) override;

        struct DebugPoint
        {
            math::Vector position;
            mono::Color::RGBA color;
            float size;
            uint32_t timestamp;
        };

        struct DebugLine
        {
            math::Vector start;
            math::Vector end;
            mono::Color::RGBA color;
            float width;
            uint32_t timestamp;
        };

        mutable std::vector<DebugPoint> m_debug_points;
        mutable std::vector<DebugLine> m_debug_lines;
    };
}
