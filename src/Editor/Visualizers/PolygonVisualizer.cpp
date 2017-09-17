
#include "PolygonVisualizer.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Texture/TextureFactory.h"
#include "Objects/Polygon.h"

using namespace editor;

PolygonVisualizer::PolygonVisualizer(const std::vector<Polygon>& polygons)
    : m_polygons(polygons)
{
    m_texture = mono::CreateTexture("res/textures/placeholder.png");
}

void PolygonVisualizer::doDraw(mono::IRenderer& renderer) const
{
    for(const Polygon& polygon : m_polygons)
    {
        constexpr mono::Color::RGBA line_color(1.0f, 0.0f, 0.7f);
        constexpr mono::Color::RGBA point_color(0.0f, 1.0f, 0.7f);
        constexpr mono::Color::RGBA selected_color(0.8f, 0.8f, 0.8f);

        std::vector<unsigned short> indices;
        for(size_t index = 0; index < polygon.vertices.size(); ++index)
            indices.push_back(index);

        renderer.DrawGeometry(polygon.vertices, polygon.texture_coordinates, indices, m_texture);

        renderer.DrawClosedPolyline(polygon.vertices, line_color, 2.0f);
        renderer.DrawPoints(polygon.vertices, point_color, 3.0f);

        renderer.DrawPoints({ polygon.centroid }, point_color, 4.0f);

        /*
        if(m_selected)
        {
            const math::Quad& bb = LocalBoundingBox();
            renderer.DrawQuad(bb, selected_color, 1.0f);
        }
         */
    }
}

math::Quad PolygonVisualizer::BoundingBox() const
{
    return math::Quad(-math::INF, -math::INF, math::INF, math::INF);
}




PolygonVisualizer2::PolygonVisualizer2(const std::vector<math::Vector>& points, const math::Vector& mouse_position)
    : m_points(points),
      m_mousePosition(mouse_position)
{
    m_texture = mono::CreateTexture("res/textures/placeholder.png");
}

void PolygonVisualizer2::doDraw(mono::IRenderer& renderer) const
{
    if(m_points.empty())
        return;

    std::vector<math::Vector> texture_points;
    texture_points.resize(m_points.size());

    DrawPolygon(renderer, m_texture, m_points, texture_points);

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 0.0f, 0.2f);
    const std::vector<math::Vector>& line = { m_points.back(), m_mousePosition, m_points.front() };
    renderer.DrawPolyline(line, color, 2.0f);
}

math::Quad PolygonVisualizer2::BoundingBox() const
{
    return math::Quad(-math::INF, -math::INF, math::INF, math::INF);
}

