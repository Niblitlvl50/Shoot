
#include "SnapperVisualizer.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"
#include "Math/Quad.h"
#include "SnapPoint.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"

#include <cmath>

using namespace editor;

SnapperVisualizer::SnapperVisualizer(const bool& draw_snappers, const std::vector<SnapPoint>& snappers)
    : m_draw_snappers(draw_snappers)
    , m_snappers(snappers)
{ }

void SnapperVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!m_draw_snappers || m_snappers.empty())
        return;

    std::vector<math::Vector> points;
    points.reserve(m_snappers.size());

    std::vector<math::Vector> line_points;
    line_points.reserve(m_snappers.size() * 6);

    for(const SnapPoint& snapper : m_snappers)
    {
        points.push_back(snapper.position);

        const math::Vector& normal_vector = math::VectorFromAngle(-snapper.normal);
        const math::Vector& left_arrow = math::VectorFromAngle(-snapper.normal + math::PI() - math::PI_4());
        const math::Vector& right_arrow = math::VectorFromAngle(-snapper.normal + math::PI() + math::PI_4());

        line_points.push_back(snapper.position);
        line_points.push_back(snapper.position + normal_vector * -0.25f);
        line_points.push_back(snapper.position + left_arrow * 0.1f);
        line_points.push_back(snapper.position);
        line_points.push_back(snapper.position);
        line_points.push_back(snapper.position + right_arrow * 0.1f);
    }

    constexpr mono::Color::RGBA line_color(1.0f, 0.0f, 0.0f);
    constexpr mono::Color::RGBA point_color(1.0f, 0.5f, 0.0f);
    renderer.DrawLines(line_points, line_color, 2.0f);
    renderer.DrawPoints(points, point_color, 4.0f);
}

math::Quad SnapperVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
