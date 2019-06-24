
#include "PhysicsStatsElement.h"
#include "FontIds.h"

#include "Math/Vector.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"

#include <cstdio>

using namespace game;

PhysicsStatsElement::PhysicsStatsElement(mono::PhysicsSystem* physics_system, const math::Vector& position, const mono::Color::RGBA& color)
    : m_physics_system(physics_system)
    , m_color(color)
{
    m_position = position;
}

void PhysicsStatsElement::Draw(mono::IRenderer& renderer) const
{
    const mono::PhysicsSystemStats& stats = m_physics_system->GetStats();

    char text[1024] = { 0 };
    std::snprintf(text, 1024, "bodies: %u shapes: %u constraints %u | circles %u segments %u polygons %u | pivots %u gears %u springs %u",
        stats.bodies, stats.shapes, stats.constraints,
        stats.circle_shapes, stats.segment_shapes, stats.polygon_shapes,
        stats.pivot_joints, stats.gear_joints, stats.damped_springs);

    renderer.DrawText(game::PIXELETTE_LARGE, text, math::ZeroVec, false, m_color);
}

void PhysicsStatsElement::Update(const mono::UpdateContext& update_context)
{ }
