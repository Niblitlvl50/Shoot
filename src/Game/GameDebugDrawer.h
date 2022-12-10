
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
        void DrawCircle(const math::Vector& position, float radius, const mono::Color::RGBA& color) override;
        void DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) override;
        void DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) override;

        void DrawPointFading(const math::Vector& position, float size, const mono::Color::RGBA& color, float time_s) override;
        void DrawLineFading(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color, float time_s) override;
        void DrawLineFading(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color, float time_s) override;
        void DrawCircleFading(const math::Vector& position, float radius, const mono::Color::RGBA& color, float time_s) override;
        void DrawScreenTextFading(const char* text, const math::Vector& position, const mono::Color::RGBA& color, float time_s) override;
        
        struct DebugPoint
        {
            math::Vector position;
            mono::Color::RGBA color;
            float size;
            float time_to_live_s;
        };

        struct DebugLine
        {
            std::vector<math::Vector> points;
            mono::Color::RGBA color;
            float width;
            float time_to_live_s;
        };

        struct DebugText
        {
            math::Vector position;
            mono::Color::RGBA color;
            float time_to_live_s;
            std::string text;
        };

        mutable std::vector<DebugPoint> m_debug_points;
        mutable std::vector<DebugLine> m_debug_lines;
        mutable std::vector<DebugText> m_debug_texts_world;
        mutable std::vector<DebugText> m_debug_texts_screen;
    };
}
