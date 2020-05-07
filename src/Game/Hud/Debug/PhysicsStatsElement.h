
#pragma once

#include "Zone/EntityBase.h"
#include "Rendering/Color.h"

namespace mono
{
    class PhysicsSystem;
}

namespace game
{
    class PhysicsStatsElement : public mono::EntityBase
    {
    public:

        PhysicsStatsElement(mono::PhysicsSystem* physics_system, const math::Vector& position, const mono::Color::RGBA& color);
        void Draw(mono::IRenderer& renderer) const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        mono::PhysicsSystem* m_physics_system;
        mono::Color::RGBA m_color;
    };
}