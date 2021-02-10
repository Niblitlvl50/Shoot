
#include "InteractionSystemDrawer.h"
#include "InteractionSystem.h"

#include "Math/Quad.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

InteractionSystemDrawer::InteractionSystemDrawer(
    InteractionSystem* interaction_system, mono::TransformSystem* transform_system)
    : m_interaction_system(interaction_system)
    , m_transform_system(transform_system)
{ }

void InteractionSystemDrawer::Draw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> interaction_points;
    std::vector<math::Vector> triggered_points;

    for(uint32_t interaction_id : m_interaction_system->GetActiveInteractions())
    {
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(interaction_id);
        const math::Vector& interaction_position = math::TopLeft(world_bb);

        interaction_points.push_back(interaction_position);
    }

    for(uint32_t interaction_id : m_interaction_system->GetTriggeredInteractions())
    {
        const math::Quad& world_bb = m_transform_system->GetWorldBoundingBox(interaction_id);
        const math::Vector& interaction_position = math::TopRight(world_bb);
        triggered_points.push_back(interaction_position);
    }

    renderer.DrawPoints(interaction_points, mono::Color::RED, 4.0f);
    renderer.DrawPoints(triggered_points, mono::Color::BLUE, 4.0f);
}

math::Quad InteractionSystemDrawer::BoundingBox() const
{
    return math::InfQuad;
}
