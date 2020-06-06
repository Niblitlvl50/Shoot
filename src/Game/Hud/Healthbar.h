
#pragma once

#include "Rendering/IDrawable.h"

namespace mono
{
    class TransformSystem;
    class EntitySystem;
}

namespace game
{
    class DamageSystem;

    class HealthbarDrawer : public mono::IDrawable
    {
    public:
        HealthbarDrawer(
            game::DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::EntitySystem* entity_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        game::DamageSystem* m_damage_system;
        mono::TransformSystem* m_transform_system;
        mono::EntitySystem* m_entity_system;
    };
}
