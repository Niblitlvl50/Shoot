
#pragma once

#include "Zone/EntityBase.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class TrailEffect : public mono::EntityBase
    {
    public:

        TrailEffect(const math::Vector& position);
        virtual ~TrailEffect();

        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;
        math::Quad BoundingBox() const override;

        const math::Vector& m_position;
        std::unique_ptr<mono::ParticlePool> m_pool;
        std::unique_ptr<mono::ParticleEmitter> m_emitter;
        std::unique_ptr<mono::ParticleDrawer> m_drawer;
    };
}
