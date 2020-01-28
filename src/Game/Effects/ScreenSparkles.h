
#pragma once

#include "Math/Quad.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ScreenSparkles
    {
    public:

        ScreenSparkles(mono::ParticleSystem* particle_system, const math::Quad& viewport);
        ~ScreenSparkles();

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
