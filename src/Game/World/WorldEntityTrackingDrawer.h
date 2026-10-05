
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"

#include "Rendering/RenderBuffer/IRenderBuffer.h"
#include "Rendering/Sprite/ISpriteFactory.h"
#include "Rendering/Sprite/SpriteBufferFactory.h"
#include "Rendering/Primitives/PrimitiveBufferFactory.h"
#include "WorldEntityTrackingSystem.h"

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

        // Indexed by EntityType, a type without a sprite in the config only gets the circle.
        mono::ISpritePtr m_type_sprites[N_ENTITY_TYPES];
        mono::SpriteDrawBuffers m_type_sprite_buffers[N_ENTITY_TYPES];

        std::unique_ptr<mono::IElementBuffer> m_sprite_indices;

        mono::PrimitiveDrawBuffers m_circle_draw_buffers;
        mono::PrimitiveDrawBuffers m_circle_outline_draw_buffers;
    };
}
