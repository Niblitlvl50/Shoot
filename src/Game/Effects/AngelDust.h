
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include <cstdint>

namespace game
{
    class AngelDust
    {
    public:

        AngelDust(
            mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system, const math::Quad& area);
        ~AngelDust();

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
