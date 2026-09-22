
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Effects/IParticleEffect.h"
#include <cstdint>

namespace game
{
    // A short burst of metal sparks, meant for wheels grinding against the rails when
    // cornering too fast.
    class WheelGrindEffect : public IParticleEffect
    {
    public:

        WheelGrindEffect(mono::ParticleSystem* particle_system, mono::IEntityManager* entity_system);
        ~WheelGrindEffect();

        void EmitAt(const math::Vector& world_position) override;
        void EmitAtWithDirection(const math::Vector& world_position, float direction) override;

    private:
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;
    };
}
