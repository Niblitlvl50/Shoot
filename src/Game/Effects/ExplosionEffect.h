
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include <cstdint>

namespace game
{
    class ExplosionEffect
    {
    public:

        ExplosionEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~ExplosionEffect();
        void ExplodeAt(const math::Vector& position);

    private:

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
