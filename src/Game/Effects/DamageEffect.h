
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include <cstdint>

namespace game
{
    class DamageEffect
    {
    public:

        DamageEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~DamageEffect();
        void EmitGibsAt(const math::Vector& position, float direction);

    private:

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}