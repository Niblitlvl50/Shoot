
#pragma once

#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"
#include <vector>

namespace game
{
    class IDebugDrawer
    {
    public:

        virtual ~IDebugDrawer() = default;

        virtual void DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color) = 0;
        virtual void DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color) = 0;
        virtual void DrawLine(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color) = 0;
        virtual void DrawCircle(const math::Vector& position, float radius, const mono::Color::RGBA& color) = 0;
        virtual void DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) = 0;
        virtual void DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) = 0;

        virtual void DrawLineFading(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color, float time_s) = 0;
        virtual void DrawLineFading(const std::vector<math::Vector>& polyline, float width, const mono::Color::RGBA& color, float time_s) = 0;
        virtual void DrawScreenTextFading(const char* text, const math::Vector& position, const mono::Color::RGBA& color, float time_s) = 0;
    };
}
