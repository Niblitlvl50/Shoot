
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class DamageSystem;

    class HealthbarDrawer : public mono::IDrawable
    {
    public:
        HealthbarDrawer(
            game::DamageSystem* damage_system, mono::TransformSystem* transform_system, mono::EntityManager* entity_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        game::DamageSystem* m_damage_system;
        mono::TransformSystem* m_transform_system;
        mono::EntityManager* m_entity_system;
    };
}
