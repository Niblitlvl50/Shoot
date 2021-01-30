
#include "PhysicsStatsElement.h"
#include "Physics/PhysicsSystem.h"

#include "Math/Quad.h"
#include "GameDebug.h"

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
    ImGui::Text("bodies: %u shapes: %u constraints %u", stats.bodies, stats.shapes, stats.constraints);
    ImGui::Text("circles %u segments %u polygons %u", stats.circle_shapes, stats.segment_shapes, stats.polygon_shapes);
    ImGui::Text("pivots %u gears %u springs %u", stats.pivot_joints, stats.gear_joints, stats.damped_springs);
    ImGui::End();
}

math::Quad PhysicsStatsElement::BoundingBox() const
{
    return math::InfQuad;
}
