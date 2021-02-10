
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class InteractionSystemDrawer : public mono::IDrawable
    {
    public:

        InteractionSystemDrawer(class InteractionSystem* interaction_system, mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        InteractionSystem* m_interaction_system;
        mono::TransformSystem* m_transform_system;
    };
}
