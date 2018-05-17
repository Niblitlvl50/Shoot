
#include "Path.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"

#include <cmath>

using namespace editor;

namespace
{
    std::vector<math::Vector> GenerateArrows(const std::vector<math::Vector>& points)
    {
        std::vector<math::Vector> vertices;
        vertices.reserve(points.size() * 4);

        for(size_t index = 1; index < points.size(); ++index)
        {
            const math::Vector& first = points[index -1];
            const math::Vector& second = points[index];

            const float angle = math::AngleBetweenPoints(first, second);

            const float x1 = std::sin(angle + math::PI() + math::PI_4()) + second.x;
            const float y1 = -std::cos(angle + math::PI() + math::PI_4()) + second.y;

            const float x2 = std::sin(angle - math::PI_4()) + second.x;
            const float y2 = -std::cos(angle - math::PI_4()) + second.y;

            vertices.emplace_back(x1, y1);
            vertices.emplace_back(second);
            vertices.emplace_back(second);
            vertices.emplace_back(x2, y2);
        }

        return vertices;
    }

    constexpr mono::Color::RGBA line_color(0.0f, 0.0f, 0.0f, 0.4f);
    constexpr mono::Color::RGBA arrow_color(0.0f, 1.0f, 0.7f, 0.4f);
    constexpr mono::Color::RGBA selected_color(0.0f, 1.0f, 0.0f);
}

PathEntity::PathEntity(const std::string& name)
    : m_name(name),
      m_selected(false)
{ }

PathEntity::PathEntity(const std::string& name, const std::vector<math::Vector>& local_points)
    : PathEntity(name)
{
    m_points = local_points;
    m_pivot_point = math::CentroidOfPolygon(m_points);
}

void PathEntity::Draw(mono::IRenderer& renderer) const
{
    if(m_selected)
        renderer.DrawPolyline(m_points, selected_color, 4.0f);

    DrawPath(renderer, m_points);
    renderer.DrawPoints( { m_pivot_point }, arrow_color, 4.0f);
}

void PathEntity::Update(unsigned int delta)
{ }

math::Quad PathEntity::BoundingBox() const
{
    const math::Matrix& local_to_world = Transformation();
    math::Quad bb(math::INF, math::INF, -math::INF, -math::INF);
    for(auto& point : m_points)
        bb |= math::Transform(local_to_world, point);

    return bb;
}

void PathEntity::SetSelected(bool selected)
{
    m_selected = selected;
}

void PathEntity::SetVertex(const math::Vector& world_point, size_t index)
{
    const math::Matrix& world_to_local = math::Inverse(Transformation());
    m_points[index] = math::Transform(world_to_local, world_point);
}

void PathEntity::SetName(const char* new_name)
{
    m_name = new_name;
}

void editor::DrawPath(mono::IRenderer& renderer, const std::vector<math::Vector>& points)
{
    renderer.DrawPolyline(points, line_color, 2.0f);

    const std::vector<math::Vector>& arrow_vertices = GenerateArrows(points);
    renderer.DrawLines(arrow_vertices, arrow_color, 2.0f);
}
