
#pragma once

#include "IDebugDrawer.h"
#include "Rendering/IDrawable.h"

#include "Math/Vector.h"
#include "Rendering/Color.h"

#include <vector>
#include <string>

namespace game
{
    class GameDebugDrawer : public IDebugDrawer, public mono::IDrawable
    {
    public:

        GameDebugDrawer();
        ~GameDebugDrawer();

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color) override;
        void DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color) override;
        void DrawLine(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color) override;
        void DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) override;
        void DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) override;

        void DrawLineFading(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color, int time) override;
        void DrawLineFading(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color, int time) override;
        void DrawScreenTextFading(const char* text, const math::Vector& position, const mono::Color::RGBA& color, int time) override;
        
        struct DebugPoint
        {
            math::Vector position;
            mono::Color::RGBA color;
            float size;
            int time_to_live;
        };

        struct DebugLine
        {
            std::vector<math::Vector> points;
            mono::Color::RGBA color;
            float width;
            int time_to_live;
        };

        struct DebugText
        {
            math::Vector position;
            mono::Color::RGBA color;
            int time_to_live;
            std::string text;
        };

        mutable std::vector<DebugPoint> m_debug_points;
        mutable std::vector<DebugLine> m_debug_lines;
        mutable std::vector<DebugText> m_debug_texts_world;
        mutable std::vector<DebugText> m_debug_texts_screen;
    };
}
