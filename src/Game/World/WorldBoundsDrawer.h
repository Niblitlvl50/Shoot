
#pragma once

#include "Rendering/IDrawable.h"
#include "MonoFwd.h"

namespace game
{
    class WorldBoundsSystem;

    class WorldBoundsDrawer : public mono::IDrawable
    {
    public:

        WorldBoundsDrawer(const mono::TransformSystem* transform_system, const WorldBoundsSystem* world_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
        const WorldBoundsSystem* m_world_system;
    };
}
