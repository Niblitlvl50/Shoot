
#pragma once

#include "Entity/EntityBase.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ParticleExplosion : public mono::EntityBase
    {
    public:
    
        ParticleExplosion(const math::Vector& position);
        virtual ~ParticleExplosion();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(unsigned int delta) override;

        std::unique_ptr<mono::ParticlePool> m_pool1;
        std::unique_ptr<mono::ParticlePool> m_pool2;
        std::unique_ptr<mono::ParticlePool> m_pool3;
        std::unique_ptr<mono::ParticleEmitter> m_emitter1;
        std::unique_ptr<mono::ParticleEmitter> m_emitter2;
        std::unique_ptr<mono::ParticleEmitter> m_emitter3;
        std::unique_ptr<mono::ParticleDrawer> m_drawer1;
        std::unique_ptr<mono::ParticleDrawer> m_drawer2;
        std::unique_ptr<mono::ParticleDrawer> m_drawer3;
    };
}
