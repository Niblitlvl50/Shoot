
#pragma once

#include "Entity/EntityBase.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class SmokeEffect : public mono::EntityBase
    {
    public:

        SmokeEffect(const math::Vector& position);
        virtual ~SmokeEffect();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

        std::unique_ptr<mono::ParticlePool> m_pool;
        std::unique_ptr<mono::ParticleEmitter> m_emitter;
        std::unique_ptr<mono::ParticleDrawer> m_drawer;
    };
}
