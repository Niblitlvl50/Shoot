
#include "SelectionVisualizer.h"
#include "Math/Quad.h"
#include "Math/MathFunctions.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "TransformSystem/TransformSystem.h"

#include "Rendering/RenderBuffer/BufferFactory.h"
#include "Util/Algorithm.h"

#include <limits>


namespace
{
    void BuildQuad(
        const math::Quad& selection_bb,
        const mono::Color::RGBA& color,
        std::vector<math::Vector>& out_vertices,
        std::vector<mono::Color::RGBA>& out_colors,
        std::vector<uint16_t>& out_indices)
    {
        const uint32_t index_offset = out_vertices.size();

        out_vertices.push_back(math::BottomLeft(selection_bb));
        out_vertices.push_back(math::TopLeft(selection_bb));
        out_vertices.push_back(math::TopRight(selection_bb));
        out_vertices.push_back(math::BottomRight(selection_bb));

        out_colors.push_back(color);
        out_colors.push_back(color);
        out_colors.push_back(color);
        out_colors.push_back(color);

        out_indices.push_back(index_offset + 0);
        out_indices.push_back(index_offset + 1);
        out_indices.push_back(index_offset + 1);
        out_indices.push_back(index_offset + 2);
        out_indices.push_back(index_offset + 2);
        out_indices.push_back(index_offset + 3);
        out_indices.push_back(index_offset + 3);
        out_indices.push_back(index_offset + 0);
    }

    constexpr uint32_t NO_SELECTION = std::numeric_limits<uint32_t>::max();
    constexpr uint32_t SELECTION_FADE_TIMER = 1000;
}

using namespace editor;

SelectionVisualizer::SelectionVisualizer(
    const std::vector<uint32_t>& selected_ids, const uint32_t& preselection_id, mono::TransformSystem* transform_system)
    : m_selected_ids(selected_ids)
    , m_preselection_id(preselection_id)
    , m_transform_system(transform_system)
    , m_selection_point_timer(SELECTION_FADE_TIMER)
    , m_selection_box_timer(SELECTION_FADE_TIMER)
{ }

void SelectionVisualizer::SetClickPoint(const math::Vector& world_point)
{
    m_selection_point = world_point;
    m_selection_point_timer = 0;
}

void SelectionVisualizer::SetSelectionBox(const math::Quad& world_bb)
{
    m_selection_box = world_bb;
    m_selection_box_timer = 0;
}

void SelectionVisualizer::Draw(mono::IRenderer& renderer) const
{
    const uint32_t n_boxes = m_selected_ids.size() + 1 + 1 + 1;

    std::vector<math::Vector> vertices;
    vertices.reserve(n_boxes * 4);

    std::vector<mono::Color::RGBA> colors;
    colors.reserve(n_boxes * 4);

    std::vector<uint16_t> indices;
    indices.reserve(n_boxes * 8);

    for(uint32_t selected_id : m_selected_ids)
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(selected_id);
        BuildQuad(selection_bb, mono::Color::WHITE, vertices, colors, indices);
    }

    if(m_preselection_id != NO_SELECTION)
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(m_preselection_id);
        BuildQuad(selection_bb, mono::Color::CYAN, vertices, colors, indices);
    }

    if(m_selection_point_timer < SELECTION_FADE_TIMER)
    {
        const math::Vector offset = { 0.1f, 0.1f };
        const math::Quad box = { m_selection_point - offset, m_selection_point + offset };

        const float scale = 1.0f - math::Scale01(m_selection_point_timer, 0u, SELECTION_FADE_TIMER);
        mono::Color::RGBA color = mono::Color::WHITE;
        color.alpha = scale;

        BuildQuad(box, color, vertices, colors, indices);
        m_selection_point_timer += renderer.GetDeltaTimeMS();
    }

    if(m_selection_box_timer < SELECTION_FADE_TIMER)
    {
        const float scale = 1.0f - math::Scale01(m_selection_box_timer, 0u, SELECTION_FADE_TIMER);
        mono::Color::RGBA color = mono::Color::WHITE;
        color.alpha = scale;

        BuildQuad(m_selection_box, color, vertices, colors, indices);
        m_selection_box_timer += renderer.GetDeltaTimeMS();
    }

    if(!vertices.empty())
    {
        std::unique_ptr<mono::IRenderBuffer> vertices_buffer =
            mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 2, vertices.size(), vertices.data());
        std::unique_ptr<mono::IRenderBuffer> colors_buffer =
            mono::CreateRenderBuffer(mono::BufferType::STATIC, mono::BufferData::FLOAT, 4, colors.size(), colors.data());
        std::unique_ptr<mono::IElementBuffer> index_buffer
            = mono::CreateElementBuffer(mono::BufferType::STATIC, indices.size(), indices.data());

        renderer.DrawLines(vertices_buffer.get(), colors_buffer.get(), index_buffer.get(), 0, index_buffer->Size());
    }
}

math::Quad SelectionVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
