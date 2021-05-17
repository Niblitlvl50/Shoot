
#include "GrabberVisualizer.h"
#include "Math/Quad.h"
#include "Grabber.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "Rendering/RenderBuffer/BufferFactory.h"

using namespace editor;

GrabberVisualizer::GrabberVisualizer(const std::vector<editor::Grabber>& grabbers)
    : m_grabbers(grabbers)
{ }

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

    std::unique_ptr<mono::IRenderBuffer> vertex_buffer = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, points.size(), points.data());
    std::unique_ptr<mono::IRenderBuffer> colors_buffer = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, colors.size(), colors.data());

    renderer.DrawPoints(vertex_buffer.get(), colors_buffer.get(), 20.0f, 0, points.size());
}

math::Quad GrabberVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
