
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

namespace game
{
    class SpawnSystemDebugDrawer : public mono::IDrawable
    {
    public:

        SpawnSystemDebugDrawer(class SpawnSystem* spawn_system, mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        SpawnSystem* m_spawn_system;
        mono::TransformSystem* m_transform_system;
    };
}
