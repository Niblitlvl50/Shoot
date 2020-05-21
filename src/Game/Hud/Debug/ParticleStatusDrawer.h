
#pragma once

#include "Zone/EntityBase.h"

namespace mono
{
    class ParticleSystem;
}

namespace game
{
    class ParticleStatusDrawer : public mono::EntityBase
    {
    public:

        ParticleStatusDrawer(const mono::ParticleSystem* particle_system, const math::Vector& position);
        void EntityDraw(mono::IRenderer& renderer) const override;
        void EntityUpdate(const mono::UpdateContext& update_context) override;

        const mono::ParticleSystem* m_particle_system;
    };
}
