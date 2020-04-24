
#pragma once

#include "Zone/EntityBase.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class ParticleExplosion : public mono::EntityBase
    {
    public:
    
        ParticleExplosion(const math::Vector& position);
        virtual ~ParticleExplosion();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;
    };
}
