
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class BlinkEffect
    {
    public:

        BlinkEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~BlinkEffect();

        void EmitBlinkAwayAt(const math::Vector& position);
        void EmitBlinkBackAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
