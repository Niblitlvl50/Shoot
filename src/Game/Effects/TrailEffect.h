
#pragma once

#include "MonoFwd.h"
#include <cstdint>

namespace game
{
    class TrailEffect
    {
    public:

        TrailEffect(
            mono::TransformSystem* transform_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system,
            uint32_t follow_id);
        ~TrailEffect();

    private:

        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
