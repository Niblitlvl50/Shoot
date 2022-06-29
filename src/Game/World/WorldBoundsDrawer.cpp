
#include "WorldBoundsDrawer.h"
#include "WorldBoundsSystem.h"

#include "TransformSystem/TransformSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"

using namespace game;

WorldBoundsDrawer::WorldBoundsDrawer(const mono::TransformSystem* transform_system, const WorldBoundsSystem* world_system)
    : m_transform_system(transform_system)
    , m_world_system(world_system)
{ }

void WorldBoundsDrawer::Draw(mono::IRenderer& renderer) const
{
    const auto draw_world_bounds = [this, &renderer](const WorldBoundsComponent& component, uint32_t index) {
        const math::Matrix world_transform = m_transform_system->GetWorld(index);
        const auto transform_scope = mono::MakeTransformScope(world_transform, &renderer);
        renderer.DrawClosedPolyline(component.vertices, mono::Color::RED, 1.0f);
    };
    m_world_system->ForEachComponent(draw_world_bounds);
}

math::Quad WorldBoundsDrawer::BoundingBox() const
{
    return math::InfQuad;
}
