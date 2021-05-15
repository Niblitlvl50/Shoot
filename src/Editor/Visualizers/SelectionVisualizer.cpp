
#include "SelectionVisualizer.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "TransformSystem/TransformSystem.h"

#include "Rendering/RenderBuffer/BufferFactory.h"

#include <limits>


namespace
{
    int BuildQuad(
        const math::Quad& selection_bb,
        const mono::Color::RGBA& color,
        uint32_t buffer_offset,
        math::Vector* out_vertices,
        mono::Color::RGBA* out_colors,
        uint16_t* out_indices)
    {
        out_vertices[buffer_offset + 0] = math::BottomLeft(selection_bb);
        out_vertices[buffer_offset + 1] = math::TopLeft(selection_bb);
        out_vertices[buffer_offset + 2] = math::TopRight(selection_bb);
        out_vertices[buffer_offset + 3] = math::BottomRight(selection_bb);

        out_colors[buffer_offset + 0] =
        out_colors[buffer_offset + 1] =
        out_colors[buffer_offset + 2] =
        out_colors[buffer_offset + 3] = color;

        const uint32_t index_offset = buffer_offset * 2;

        out_indices[index_offset + 0] = buffer_offset + 0;
        out_indices[index_offset + 1] = buffer_offset + 1;
        out_indices[index_offset + 2] = buffer_offset + 1;
        out_indices[index_offset + 3] = buffer_offset + 2;
        out_indices[index_offset + 4] = buffer_offset + 2;
        out_indices[index_offset + 5] = buffer_offset + 3;
        out_indices[index_offset + 6] = buffer_offset + 3;
        out_indices[index_offset + 7] = buffer_offset + 0;

        return 4;
    }
}

using namespace editor;

SelectionVisualizer::SelectionVisualizer(
    const std::vector<uint32_t>& selected_ids, const uint32_t& preselection_id, mono::TransformSystem* transform_system)
    : m_selected_ids(selected_ids)
    , m_preselection_id(preselection_id)
    , m_transform_system(transform_system)
{
    m_vertices = mono::CreateRenderBuffer(mono::BufferType::DYNAMIC, mono::BufferData::FLOAT, 2, 8, nullptr);
    m_colors = mono::CreateRenderBuffer(mono::BufferType::DYNAMIC, mono::BufferData::FLOAT, 4, 8, nullptr);
    m_indices = mono::CreateElementBuffer(mono::BufferType::DYNAMIC, 16, nullptr);
}

void SelectionVisualizer::Draw(mono::IRenderer& renderer) const
{
    math::Vector vertices[8];
    mono::Color::RGBA colors[8];
    uint16_t indices[16];

    uint32_t count = 0;

    if(m_preselection_id != std::numeric_limits<uint32_t>::max())
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(m_preselection_id);
        count += BuildQuad(selection_bb, mono::Color::BLUE, count, vertices, colors, indices);
    }

    for(uint32_t selected_id : m_selected_ids)
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(selected_id);
        count += BuildQuad(selection_bb, mono::Color::RED, count, vertices, colors, indices);
    }

    if(count == 0)
        return;

    m_vertices->UpdateData(vertices, 0, std::size(vertices));
    m_colors->UpdateData(colors, 0, std::size(colors));
    m_indices->UpdateData(indices, 0, std::size(indices));

    renderer.DrawLines(m_vertices.get(), m_colors.get(), m_indices.get(), 0, count * 2);
}

math::Quad SelectionVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
