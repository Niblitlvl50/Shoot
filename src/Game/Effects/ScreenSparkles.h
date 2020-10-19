
#pragma once

#include "Math/Quad.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ScreenSparkles
    {
    public:

        ScreenSparkles(mono::ParticleSystem* particle_system, const math::Vector& camera_position, const math::Vector& viewport_size);
        ~ScreenSparkles();

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
