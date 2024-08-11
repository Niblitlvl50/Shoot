
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Sprite/ISpriteFactory.h"

#include "System/Audio.h"

#include <cstdint>

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

        mutable mono::ISpritePtr m_item_selection_sprite;
        mono::SpriteDrawBuffers m_item_selection_sprite_buffer;

        mutable mono::ISpritePtr m_cursor_sprite;
        mono::SpriteDrawBuffers m_cursor_sprite_buffer;

        std::unique_ptr<mono::IElementBuffer> m_indices;

        audio::ISoundPtr m_select_sound;
        mutable uint32_t m_last_active_item_index;
    };
}
