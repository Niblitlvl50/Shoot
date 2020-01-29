
#pragma once

#include "Math/Vector.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class MuzzleFlash
    {
    public:

        MuzzleFlash(mono::ParticleSystem* particle_system);
        ~MuzzleFlash();
        void EmittAt(const math::Vector& position, float direction);

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
