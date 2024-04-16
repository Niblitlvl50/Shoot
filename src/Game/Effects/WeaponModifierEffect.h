
#pragma once

#include "MonoFwd.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>

namespace game
{
    class WeaponModifierEffect
    {
    public:

        WeaponModifierEffect(
            mono::TransformSystem* transform_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system);
        ~WeaponModifierEffect();

        void AttachToEntityWithOffset(uint32_t entity_id, const math::Vector& offset);
        void EmitForDuration(uint32_t entity_id, float duration_s);

    private:

        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
