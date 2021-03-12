
#pragma once

#include "IUpdatable.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Texture/ITextureFactory.h"

namespace game
{
    class FogOverlay : public mono::IUpdatable, public mono::IDrawable
    {
    public:

        FogOverlay();
        ~FogOverlay();

        void Update(const mono::UpdateContext& context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        mono::ITexturePtr m_texture;
        std::unique_ptr<mono::IRenderBuffer> m_vertex_buffer;
        std::unique_ptr<mono::IElementBuffer> m_index_buffer;
    };
}
