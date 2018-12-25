
#include "GridVisualizer.h"
#include "Camera/ICamera.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

namespace
{
    std::vector<math::Vector> BuildGridVertices(const math::Quad& bounds)
    {
        constexpr float cellSize = 50.0f;

        const float width = bounds.mB.x - bounds.mA.x;
        const float height = bounds.mB.y - bounds.mA.y;

        const float cellsWidth = width / cellSize;
        const float cellsHeight = height / cellSize;

        std::vector<math::Vector> vertices;

        for(int index = 0; index < cellsWidth; ++index)
        {
            const float x = bounds.mA.x + index * cellSize;

            vertices.push_back(math::Vector(x, bounds.mA.y));
            vertices.push_back(math::Vector(x, bounds.mB.y));
        }

        for(int index = 0; index < cellsHeight; ++index)
        {
            const float y = bounds.mA.y + index * cellSize;

            vertices.push_back(math::Vector(bounds.mA.x, y));
            vertices.push_back(math::Vector(bounds.mB.x, y));
        }
        
        return vertices;
    }
}

using namespace editor;

GridVisualizer::GridVisualizer()
    : m_gridVertices(BuildGridVertices(math::Quad(0, 0, 1200, 800)))
{ }

void GridVisualizer::doDraw(mono::IRenderer& renderer) const
{
    const math::Matrix& projection = math::Ortho(0.0f, 1200, 0.0f, 800, -10.0f, 10.0f);
    constexpr math::Matrix transform;

    constexpr mono::Color::RGBA gray_color(1.0f, 1.0f, 1.0f, 0.2f);

    renderer.PushNewProjection(projection);
    renderer.PushNewTransform(transform);
    renderer.DrawLines(m_gridVertices, gray_color, 1.0f);
}

math::Quad GridVisualizer::BoundingBox() const
{
    return math::InfQuad;
}

