
#pragma once

#include "MonoFwd.h"
#include "Particle/ParticleFwd.h"
#include <cstdint>
#include <unordered_map>

namespace game
{
    class BulletTrailEffect
    {
    public:

        BulletTrailEffect(
            mono::TransformSystem* transform_system,
            mono::ParticleSystem* particle_system,
            mono::IEntityManager* entity_system);
        ~BulletTrailEffect();

        void AttachEmitterToBullet(uint32_t entity_id);
        void RemoveEmitterFromBullet(uint32_t entity_id);

    private:

        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::IEntityManager* m_entity_system;
        uint32_t m_particle_entity;

        std::unordered_map<uint32_t, mono::ParticleEmitterComponent*> m_bullet_id_to_emitter;
    };
}
