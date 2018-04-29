
#include "SnapperVisualizer.h"
#include "Math/MathFwd.h"
#include "Math/Quad.h"
#include "SnapPoint.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"

#include <cmath>

using namespace editor;

SnapperVisualizer::SnapperVisualizer(const std::vector<editor::SnapPoint>& snappers)
    : m_snappers(snappers)
{ }

void SnapperVisualizer::doDraw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> points;
    points.reserve(m_snappers.size() * 2);

    for(const SnapPoint& snapper : m_snappers)
    {
        points.push_back(snapper.position);

        const float x = std::sin(snapper.normal);
        const float y = std::cos(snapper.normal);

        points.push_back(snapper.position + math::Vector(x, y) * 0.5f);
    }

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f);
    renderer.DrawPoints(points, color, 4.0f);
    renderer.DrawLines(points, color, 1.0f);
}

math::Quad SnapperVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
