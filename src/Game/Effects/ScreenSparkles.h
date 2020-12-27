
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class ScreenSparkles
    {
    public:

        ScreenSparkles(
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system,
            const math::Vector& camera_position,
            const math::Vector& viewport_size);
        ~ScreenSparkles();

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
