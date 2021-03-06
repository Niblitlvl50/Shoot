
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class ImpactEffect
    {
    public:

        ImpactEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~ImpactEffect();
        void EmittAt(const math::Vector& position, float direction);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
