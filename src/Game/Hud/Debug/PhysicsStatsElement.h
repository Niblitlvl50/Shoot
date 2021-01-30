
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class PhysicsStatsElement : public mono::IDrawable
    {
    public:

        PhysicsStatsElement(mono::PhysicsSystem* physics_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        mono::PhysicsSystem* m_physics_system;
    };
}
