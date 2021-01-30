
#include "ParticleStatusDrawer.h"
#include "Particle/ParticleSystem.h"

#include "Math/Quad.h"
#include "GameDebug.h"

#include "imgui/imgui.h"

using namespace game;

ParticleStatusDrawer::ParticleStatusDrawer(const mono::ParticleSystem* particle_system)
    : m_particle_system(particle_system)
{ }

void ParticleStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_particle_stats)
        return;

    const mono::ParticleSystemStats& stats = m_particle_system->GetStats();

    constexpr int flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoResize;

    ImGui::Begin("particle_status", &game::g_draw_particle_stats, flags);
    ImGui::Text("pools: %u emitters: %u", stats.active_pools, stats.active_emitters);
    ImGui::End();
}

math::Quad ParticleStatusDrawer::BoundingBox() const
{
    return math::InfQuad;
}
