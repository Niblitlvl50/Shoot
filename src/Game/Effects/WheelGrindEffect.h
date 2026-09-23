
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Effects/IParticleEffect.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>

namespace game
{
    // A short burst of metal sparks, meant for wheels grinding against the rails when
    // cornering too fast.
    class WheelGrindEffect : public IParticleEffect
    {
    public:

        WheelGrindEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~WheelGrindEffect();

        void Start();
        void Stop();

        void EmitAt(const math::Vector& world_position) override;
        void EmitAtWithDirection(const math::Vector& world_position, float direction) override;

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;

        mono::ParticleEmitterComponent* m_emitter;
        float m_direction = 0.0f;
    };
}
