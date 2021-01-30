
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class ParticleStatusDrawer : public mono::IDrawable
    {
    public:

        ParticleStatusDrawer(const mono::ParticleSystem* particle_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::ParticleSystem* m_particle_system;
    };
}
