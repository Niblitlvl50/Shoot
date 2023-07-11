
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    class PickupEffect
    {
    public:

        PickupEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~PickupEffect();
        void EmitAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };


    class PickupLootEffect
    {
    public:

        PickupLootEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~PickupLootEffect();
        void EmitAt(const math::Vector& position);

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
