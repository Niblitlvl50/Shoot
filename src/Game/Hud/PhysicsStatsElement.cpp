
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

    char text_1[512] = { 0 };
    char text_2[512] = { 0 };
    char text_3[512] = { 0 };

    std::snprintf(text_1, 512, "bodies: %u shapes: %u constraints %u", stats.bodies, stats.shapes, stats.constraints);
    std::snprintf(text_2, 512, "circles %u segments %u polygons %u", stats.circle_shapes, stats.segment_shapes, stats.polygon_shapes);
    std::snprintf(text_3, 512, "pivots %u gears %u springs %u", stats.pivot_joints, stats.gear_joints, stats.damped_springs);

    renderer.DrawText(game::PIXELETTE_MEGA, text_1, math::ZeroVec, false, m_color);
    renderer.DrawText(game::PIXELETTE_MEGA, text_2, math::Vector(0.0f, -5.0f), false, m_color);
    renderer.DrawText(game::PIXELETTE_MEGA, text_3, math::Vector(0.0f, -10.0f), false, m_color);
}

void PhysicsStatsElement::Update(const mono::UpdateContext& update_context)
{ }
