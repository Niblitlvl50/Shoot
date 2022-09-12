
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"

#include "Player/PlayerInfo.h"

#include <memory>

namespace game
{
    struct AimlineRenderData
    {
        std::unique_ptr<mono::IRenderBuffer> vertices;
        std::unique_ptr<mono::IRenderBuffer> colors;
        std::unique_ptr<mono::IElementBuffer> indices;
    };

    class PlayerAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PlayerAuxiliaryDrawer(const mono::TransformSystem* transform_system);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
        mutable AimlineRenderData m_aimline_data[n_players];
        mutable float m_cooldown_position;
        mutable float m_cooldown_velocity;
    };

    class PackageAuxiliaryDrawer : public mono::IDrawable
    {
    public:

        PackageAuxiliaryDrawer(const mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const mono::TransformSystem* m_transform_system;
        mono::ISpritePtr m_package_sprite;
        mono::SpriteDrawBuffers m_sprite_buffers;
        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
