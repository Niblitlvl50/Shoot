
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>

namespace game
{
    class TrainSmokeEffect
    {
    public:

        TrainSmokeEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, mono::TransformSystem* transform_system, uint32_t parent_entity_id);
        ~TrainSmokeEffect();

        void Start();
        void Stop();
        void UpdateEmitterSpeed(float emit_rate);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;

        mono::ParticleEmitterComponent* m_emitter;
    };
}
