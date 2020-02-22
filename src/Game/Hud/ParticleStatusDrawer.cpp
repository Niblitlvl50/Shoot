
#include "ParticleStatusDrawer.h"
#include "FontIds.h"
#include "GameDebug.h"

#include "Math/Vector.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Particle/ParticleSystem.h"

#include <cstdio>

using namespace game;

ParticleStatusDrawer::ParticleStatusDrawer(const mono::ParticleSystem* particle_system, const math::Vector& position)
    : m_particle_system(particle_system)
{
    m_position = position;
}

void ParticleStatusDrawer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_particle_stats)
        return;

    const mono::ParticleSystemStats& stats = m_particle_system->GetStats();

    char text[512] = { 0 };
    std::snprintf(text, 512, "pools: %u emitters: %u", stats.active_pools, stats.active_emitters);
    renderer.DrawText(game::PIXELETTE_MEGA, text, math::ZeroVec, false, mono::Color::BLACK);
}

void ParticleStatusDrawer::Update(const mono::UpdateContext& update_context)
{ }
