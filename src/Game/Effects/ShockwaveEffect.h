
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>

namespace game
{
    class ShockwaveEffect
    {
    public:

        ShockwaveEffect(
            mono::TransformSystem* transform_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system);
        ~ShockwaveEffect();
        void EmittAt(const math::Vector& position);

    private:
        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;

        mono::ParticleEmitterComponent* m_emitter;
    };
}
