
#include "GridVisualizer.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

namespace
{
    std::vector<math::Vector> BuildGridVertices(const math::Quad& bounds)
    {
        constexpr float cell_size = 50.0f;

        const float width = math::Width(bounds);
        const float height = math::Height(bounds);

        const float cells_width = width / cell_size;
        const float cells_height = height / cell_size;

        std::vector<math::Vector> vertices;

        for(int index = 0; index < cells_width; ++index)
        {
            const float x = bounds.bottom_left.x + index * cell_size;

            vertices.push_back(math::Vector(x, bounds.bottom_left.y));
            vertices.push_back(math::Vector(x, bounds.top_right.y));
        }

        for(int index = 0; index < cells_height; ++index)
        {
            const float y = bounds.bottom_left.y + index * cell_size;

            vertices.push_back(math::Vector(bounds.bottom_left.x, y));
            vertices.push_back(math::Vector(bounds.top_right.x, y));
        }
        
        return vertices;
    }
}

using namespace editor;

GridVisualizer::GridVisualizer(const bool& draw_grid)
    : m_draw_grid(draw_grid)
{
    const std::vector<math::Vector>& grid_vertices = BuildGridVertices(math::Quad(0, 0, 1200, 800));

    constexpr mono::Color::RGBA gray_color(1.0f, 1.0f, 1.0f, 0.2f);
    const std::vector<mono::Color::RGBA> color(grid_vertices.size(), gray_color);

    m_vertices = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, grid_vertices.size(), grid_vertices.data(), "grid_draw_buffer");
    m_colors = mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, color.size(), color.data(), "grid_draw_buffer");
}

void GridVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!m_draw_grid)
        return;

    const math::Matrix& projection = math::Ortho(0.0f, 1200, 0.0f, 800, -10.0f, 10.0f);
    const mono::ScopedTransform projection_scope = mono::MakeProjectionScope(projection, &renderer);
    const mono::ScopedTransform view_scope = mono::MakeViewTransformScope(math::Matrix(), &renderer);
    const mono::ScopedTransform transform_scope = mono::MakeTransformScope(math::Matrix(), &renderer);

    renderer.DrawLines(m_vertices.get(), m_colors.get(), 0, m_vertices->Size());
}

math::Quad GridVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
