
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"

namespace game
{
    class WorldEntityTrackingSystem;

    class WorldEntityTrackingDrawer : public mono::IDrawable
    {
    public:

        WorldEntityTrackingDrawer(
            const WorldEntityTrackingSystem* entity_tracking_system, const mono::TransformSystem* transform_system);
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        const WorldEntityTrackingSystem* m_entity_tracking_system;
        const mono::TransformSystem* m_transform_system;

        mono::ISpritePtr m_package_sprite;
        mono::SpriteDrawBuffers m_package_sprite_buffers;

        mono::ISpritePtr m_boss_sprite;
        mono::SpriteDrawBuffers m_boss_sprite_buffers;

        mono::ISpritePtr m_loot_sprite;
        mono::SpriteDrawBuffers m_loot_sprite_buffers;

        std::unique_ptr<mono::IElementBuffer> m_indices;
    };
}
