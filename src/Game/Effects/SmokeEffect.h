
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class SmokeEffect
    {
    public:

        SmokeEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~SmokeEffect();
        void EmitSmokeAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
