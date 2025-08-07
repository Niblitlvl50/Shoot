
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Effects/IParticleEffect.h"
#include <cstdint>

namespace game
{
    class VampericEffect : public IParticleEffect
    {
    public:

        VampericEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~VampericEffect();

        void EmitAt(const math::Vector& world_position) override;
        void EmitAtWithDirection(const math::Vector& world_position, float direction) override;

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
