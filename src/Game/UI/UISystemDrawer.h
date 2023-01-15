
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Sprite/ISpriteFactory.h"

namespace game
{
    class UISystem;

    class UISystemDrawer : public mono::IDrawable
    {
    public:

        UISystemDrawer(const UISystem* ui_system, mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const UISystem* m_ui_system;
        const mono::TransformSystem* m_transform_system;

        mutable mono::ISpritePtr m_sprite;
        mono::SpriteDrawBuffers m_sprite_buffer;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
