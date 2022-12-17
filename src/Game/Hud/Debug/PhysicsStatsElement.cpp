
#include "PhysicsStatsElement.h"
#include "Physics/PhysicsSystem.h"

#include "Math/Quad.h"
#include "Debug/GameDebug.h"

#include "imgui/imgui.h"

using namespace game;

PhysicsStatsElement::PhysicsStatsElement(mono::PhysicsSystem* physics_system)
    : m_physics_system(physics_system)
{ }

void PhysicsStatsElement::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_physics_stats)
        return;

    const mono::PhysicsSystemStats& stats = m_physics_system->GetStats();

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::Begin("Physics Stats", &game::g_draw_physics_stats, flags);
    ImGui::Text("bodies: %u shapes: %u constraints %u",
        stats.bodies, stats.shapes, stats.constraints);
    ImGui::Text("circles %u/%u segments %u/%u polygons %u/%u",
        stats.circle_shapes, stats.circle_shapes_max,
        stats.segment_shapes, stats.segment_shapes_max,
        stats.polygon_shapes, stats.polygon_shapes_max);
    ImGui::Text("pivots %u/%u gears %u/%u springs %u/%u",
        stats.pivot_joints, stats.pivot_joints_max,
        stats.gear_joints, stats.gear_joints_max,
        stats.damped_springs, stats.damped_springs_max);
    ImGui::End();
}

math::Quad PhysicsStatsElement::BoundingBox() const
{
    return math::InfQuad;
}
