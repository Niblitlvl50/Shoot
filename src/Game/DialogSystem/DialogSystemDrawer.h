
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Rendering/Texture/TextureBufferFactory.h"

namespace game
{
    class DialogSystemDrawer : public mono::IDrawable
    {
    public:

        DialogSystemDrawer(class DialogSystem* message_system, const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        DialogSystem* m_message_system;
        const mono::TransformSystem* m_transform_system;

        mono::ITexturePtr m_texture;
        mono::TextureDrawBuffers m_draw_buffers;
    };
}
