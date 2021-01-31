
#include "GrabberVisualizer.h"
#include "Math/Quad.h"
#include "Grabber.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"

using namespace editor;

GrabberVisualizer::GrabberVisualizer(const std::vector<editor::Grabber>& grabbers)
    : m_grabbers(grabbers)
{
    m_vertices = mono::CreateRenderBuffer(mono::BufferType::DYNAMIC, mono::BufferData::FLOAT, 2, 32, nullptr);
    m_colors = mono::CreateRenderBuffer(mono::BufferType::DYNAMIC, mono::BufferData::FLOAT, 4, 32, nullptr);
}

void GrabberVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(m_grabbers.empty())
        return;

    constexpr mono::Color::RGBA default_color(1.0f, 0.5f, 0.0f, 0.8f);
    constexpr mono::Color::RGBA hoover_color(0.0f, 0.5f, 1.0f, 1.0f);

    std::vector<math::Vector> points;
    points.reserve(m_grabbers.size());

    std::vector<mono::Color::RGBA> colors;
    colors.reserve(m_grabbers.size());

    for(const Grabber& grabber : m_grabbers)
    {
        points.push_back(grabber.position);
        colors.push_back(grabber.hoover ? hoover_color : default_color);
    }

    m_vertices->UpdateData(points.data(), 0, points.size());
    m_colors->UpdateData(colors.data(), 0, colors.size());

    renderer.DrawPoints(m_vertices.get(), m_colors.get(), 10.0f, 0, points.size());
}

math::Quad GrabberVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
