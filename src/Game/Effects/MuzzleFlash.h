
#pragma once

#include "Math/Vector.h"
#include "Particle/ParticleFwd.h"
#include "Rendering/RenderFwd.h"
#include "IUpdatable.h"
#include <memory>

namespace game
{
    class MuzzleFlash
    {
    public:

        MuzzleFlash(const math::Vector& position);
        ~MuzzleFlash();
    
        void Draw(mono::IRenderer& renderer) const;
        void Update(const mono::UpdateContext& update_context);
        void SetPosition(const math::Vector& position);
    
        //std::unique_ptr<mono::ParticlePool> m_pool;
        //std::unique_ptr<mono::ParticleEmitter> m_emitter;
        //std::unique_ptr<mono::ParticleDrawer> m_drawer;
    };
}
