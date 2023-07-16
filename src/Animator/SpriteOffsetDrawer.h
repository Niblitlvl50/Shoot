
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Sprite/SpriteData.h"

namespace animator
{
    class SpriteOffsetDrawer : public mono::IDrawable
    {
    public:

        SpriteOffsetDrawer(
            mono::TransformSystem* transform_system,
            const mono::SpriteData* sprite_data,
            uint32_t sprite_id,
            const bool& offset_mode);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        mono::TransformSystem* m_transform_system;
        const mono::SpriteData* m_sprite_data;
        uint32_t m_sprite_id;
        const bool& m_offset_mode;
    };
}
