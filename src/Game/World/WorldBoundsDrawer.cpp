
#include "WorldBoundsDrawer.h"
#include "WorldBoundsSystem.h"

#include "TransformSystem/TransformSystem.h"
#include "Rendering/IRenderer.h"
#include "Math/Quad.h"

using namespace game;

WorldBoundsDrawer::WorldBoundsDrawer(const mono::TransformSystem* transform_system, const WorldBoundsSystem* world_system)
    : m_transform_system(transform_system)
    , m_world_system(world_system)
{

}

void WorldBoundsDrawer::Draw(mono::IRenderer& renderer) const
{

}

math::Quad WorldBoundsDrawer::BoundingBox() const
{
    return math::InfQuad;
}
