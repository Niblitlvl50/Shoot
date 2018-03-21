
#pragma once

#include "IUpdatable.h"
#include "Rendering/IDrawable.h"
#include "Particle/ParticleFwd.h"

#include <memory>
#include <vector>

namespace game
{
    class GibSystem : public mono::IDrawable, public mono::IUpdatable
    {
    public:

        GibSystem();
        ~GibSystem();

        void doUpdate(unsigned int delta) override;
        void doDraw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void EmitGibsAt(const math::Vector& position, float direction);
        void EmitBloodAt(const math::Vector& position, float direction);

    private:

        std::unique_ptr<mono::ParticlePool> m_pool;
        std::unique_ptr<mono::ParticleDrawer> m_drawer;

        std::vector<mono::ParticleEmitter> m_emitters;
    };
}