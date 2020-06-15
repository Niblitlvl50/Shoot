
#pragma once

#include "Math/MathFwd.h"
#include "Rendering/RenderFwd.h"

namespace game
{
    class IDebugDrawer
    {
    public:

        virtual ~IDebugDrawer() = default;

        virtual void DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color) = 0;
        virtual void DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color) = 0;
        virtual void DrawScreenText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) = 0;
        virtual void DrawWorldText(const char* text, const math::Vector& position, const mono::Color::RGBA& color) = 0;
    };
}
