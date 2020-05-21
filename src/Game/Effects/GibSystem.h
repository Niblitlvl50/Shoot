
#pragma once

#include "IUpdatable.h"
#include "Rendering/IDrawable.h"
#include "Particle/ParticleFwd.h"

namespace game
{
    class GibSystem : public mono::IDrawable, public mono::IUpdatable
    {
    public:

        GibSystem();
        ~GibSystem();

        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void EmitGibsAt(const math::Vector& position, float direction);
        void EmitBloodAt(const math::Vector& position, float direction);
    };
}