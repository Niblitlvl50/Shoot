
#pragma once

#include "Entity/EntityBase.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ScreenSparkles : public mono::EntityBase
    {
    public:

        ScreenSparkles(const math::Vector& position);
        virtual ~ScreenSparkles();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

        std::unique_ptr<mono::ParticlePool> m_pool;
        std::unique_ptr<mono::ParticleEmitter> m_emitter;
        std::unique_ptr<mono::ParticleDrawer> m_drawer;
    };
}
