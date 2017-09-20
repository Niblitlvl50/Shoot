
#include "ScaleVisualizer.h"

#include "Rendering/ICamera.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include "Math/Quad.h"
#include "Math/Matrix.h"

#include <cstdio>

using namespace editor;

ScaleVisualizer::ScaleVisualizer(const mono::ICameraPtr& camera)
    : m_camera(camera)
{ }

void ScaleVisualizer::doDraw(mono::IRenderer& renderer) const
{
    const math::Matrix& projection = math::Ortho(0.0f, 1200, 0.0f, 800, -10.0f, 10.0f);
    constexpr math::Matrix transform;

    const math::Quad& viewport = m_camera->GetViewport();
    const float scale = viewport.mB.x / 1200.0f * 200.0f;

    char text[100];
    std::sprintf(text, "%3.2f m", scale);

    constexpr math::Vector position(1050, 15);
    const std::vector<math::Vector> points = { math::Vector(950, 25), math::Vector(1150, 25) };

    constexpr mono::Color::RGBA black_color(0.0f, 0.0f, 0.0f, 0.5f);
    
    renderer.PushNewProjection(projection);
    renderer.PushNewTransform(transform);
    renderer.DrawLines(points, black_color, 2.0f);
    renderer.DrawText(0, text, position, true, black_color);
}

math::Quad ScaleVisualizer::BoundingBox() const
{
    return math::Quad(-math::INF, -math::INF, math::INF, math::INF);
}
