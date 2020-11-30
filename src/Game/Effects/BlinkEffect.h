
#pragma once

#include "Math/Vector.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class BlinkEffect
    {
    public:

        BlinkEffect(mono::ParticleSystem* particle_system);
        ~BlinkEffect();

        void EmitBlinkAwayAt(const math::Vector& position);
        void EmitBlinkBackAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        uint32_t m_particle_entity;
    };
}
