
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class HealEffect
    {
    public:

        HealEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_manager);
        ~HealEffect();
        void EmitAt(const math::Vector& position);

    private:

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_manager;
        uint32_t m_particle_entity;
    };
}
