
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Player/PlayerInfo.h"

namespace game
{
    class PlayerReloadDrawer : public mono::IDrawable
    {
    public:

        PlayerReloadDrawer(const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
    };
}
