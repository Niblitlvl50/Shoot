
#pragma once

#include "Particle/ParticleFwd.h"

namespace mono
{
    class TransformSystem;
}

namespace game
{
    class TrailEffect
    {
    public:

        TrailEffect(mono::TransformSystem* transform_system, mono::ParticleSystem* particle_system, uint32_t follow_id);
        ~TrailEffect();

    private:

        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
