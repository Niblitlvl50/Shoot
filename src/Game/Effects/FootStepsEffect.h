
#pragma once

#include "MonoFwd.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>
#include <unordered_map>

namespace game
{
    class FootStepsEffect
    {
    public:

        FootStepsEffect(
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system);
        ~FootStepsEffect();

        void EmitFootStepsAt(const math::Vector& world_position);

    private:

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
