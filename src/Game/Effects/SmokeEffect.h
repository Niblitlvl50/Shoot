
#pragma once

#include "Math/Vector.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class SmokeEffect
    {
    public:

        SmokeEffect(mono::ParticleSystem* particle_system);
        ~SmokeEffect();
        void EmitSmokeAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
