
#include "PolygonVisualizer.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Objects/Polygon.h"

using namespace editor;

PolygonVisualizer2::PolygonVisualizer2(const std::vector<math::Vector>& points, const math::Vector& mouse_position)
    : m_points(points),
      m_mousePosition(mouse_position)
{
    m_texture = mono::GetTextureFactory()->CreateTexture("res/textures/placeholder.png");
}

void PolygonVisualizer2::Draw(mono::IRenderer& renderer) const
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
    return math::InfQuad;
}

