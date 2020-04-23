
#include "GameDebugDrawer.h"
#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Util/Algorithm.h"

#include "Factories.h"

using namespace game;

namespace
{
    template <typename T>
    bool remove_if_5(const T& item)
    {
        return item.timestamp > 5000;
    }
}

GameDebugDrawer::GameDebugDrawer()
{
    game::g_debug_drawer = this;
}

GameDebugDrawer::~GameDebugDrawer()
{
    game::g_debug_drawer = nullptr;
}

void GameDebugDrawer::doDraw(mono::IRenderer& renderer) const
{
    for(DebugPoint& point : m_debug_points)
    {
        const std::vector<math::Vector> points = { point.position };
        renderer.DrawPoints(points, point.color, point.size);

        point.timestamp += renderer.GetDeltaTimeMS();
    }

    for(DebugLine& line : m_debug_lines)
    {
        const std::vector<math::Vector> line_points = { line.start, line.end };
        mono::Color::RGBA color = line.color;
        color.alpha = 1.0f - float(line.timestamp) / 5000.0f;
        renderer.DrawLines(line_points, color, line.width);

        line.timestamp += renderer.GetDeltaTimeMS();
    }

    mono::remove_if(m_debug_points, remove_if_5<DebugPoint>);
    mono::remove_if(m_debug_lines, remove_if_5<DebugLine>);
}

math::Quad GameDebugDrawer::BoundingBox() const
{
    return math::InfQuad;
}

void GameDebugDrawer::DrawPoint(const math::Vector& position, float size, const mono::Color::RGBA& color)
{
    const DebugPoint new_point = { position, color, size, 0 };
    m_debug_points.push_back(new_point);
}

void GameDebugDrawer::DrawLine(const math::Vector& start_position, const math::Vector& end_position, float width, const mono::Color::RGBA& color)
{
    const DebugLine new_line = { start_position, end_position, color, width, 0 };
    m_debug_lines.push_back(new_line);
}
