
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Effects/IParticleEffect.h"
#include <cstdint>

namespace game
{
    class ImpactEffect : public IParticleEffect
    {
    public:

        ImpactEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~ImpactEffect();

        void EmitAt(const math::Vector& world_position) override;
        void EmitAtWithDirection(const math::Vector& world_position, float direction) override;

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
