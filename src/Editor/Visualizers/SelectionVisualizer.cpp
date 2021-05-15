
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
{ }

void SelectionVisualizer::PushSelectionQuad(const math::Quad& world_bb)
{
    m_selection_boxes.push_back({ world_bb, 0 });
}

void SelectionVisualizer::Draw(mono::IRenderer& renderer) const
{
    const uint32_t n_boxes = m_selected_ids.size() + m_selection_boxes.size() + 1 + 1;

    std::vector<math::Vector> vertices;
    vertices.reserve(n_boxes * 4);

    std::vector<mono::Color::RGBA> colors;
    colors.reserve(n_boxes * 4);

    std::vector<uint16_t> indices;
    indices.reserve(n_boxes * 8);

    if(m_preselection_id != NO_SELECTION)
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(m_preselection_id);
        BuildQuad(selection_bb, mono::Color::BLUE, vertices, colors, indices);
    }

    for(uint32_t selected_id : m_selected_ids)
    {
        const math::Quad& selection_bb = m_transform_system->GetWorldBoundingBox(selected_id);
        BuildQuad(selection_bb, mono::Color::RED, vertices, colors, indices);
    }

    for(SelectionBox& box : m_selection_boxes)
    {
        const float scale = 1.0f - math::Scale01(box.timer, 0u, SELECTION_FADE_TIMER);
        mono::Color::RGBA color = mono::Color::CYAN;
        color.alpha = scale;

        BuildQuad(box.bb, color, vertices, colors, indices);
        box.timer += renderer.GetDeltaTimeMS();
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

    const auto remove_if_faded = [](const SelectionBox& box) {
        return box.timer > SELECTION_FADE_TIMER;
    };
    mono::remove_if(m_selection_boxes, remove_if_faded);
}

math::Quad SelectionVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
