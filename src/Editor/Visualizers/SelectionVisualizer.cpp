
#include "SelectionVisualizer.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"
#include "Rendering/IRenderer.h"
#include "TransformSystem/TransformSystem.h"

#include <limits>

using namespace editor;

SelectionVisualizer::SelectionVisualizer(
    const uint32_t& selection_id, const uint32_t& preselection_id, mono::TransformSystem* transform_system)
    : m_selection_id(selection_id)
    , m_preselection_id(preselection_id)
    , m_transform_system(transform_system)
{ }

void SelectionVisualizer::doDraw(mono::IRenderer& renderer) const
{
    if(m_preselection_id != std::numeric_limits<uint32_t>::max() && m_preselection_id < m_transform_system->Capacity())
    {
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_preselection_id);
        renderer.DrawQuad(world_bb, mono::Color::BLUE, 1.0f);
    }

    if(m_selection_id != std::numeric_limits<uint32_t>::max() && m_selection_id < m_transform_system->Capacity())
    {
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(m_selection_id);
        renderer.DrawQuad(world_bb, mono::Color::RED, 1.0f);
    }
}

math::Quad SelectionVisualizer::BoundingBox() const
{
    return math::InfQuad;
}
