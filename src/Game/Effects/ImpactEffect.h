
#pragma once

#include "Math/Vector.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ImpactEffect
    {
    public:

        ImpactEffect(mono::ParticleSystem* particle_system);
        ~ImpactEffect();
        void EmittAt(const math::Vector& position, float direction);

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
