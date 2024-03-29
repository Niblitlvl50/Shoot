
#pragma once

#include "MonoFwd.h"
#include "WorldBoundsTypes.h"
#include "Rendering/IDrawable.h"
#include "Rendering/RenderBuffer/IRenderBuffer.h"

#include <unordered_map>
#include <memory>

namespace game
{
    class WorldBoundsSystem;
    struct WorldBoundsComponent;

    class WorldBoundsDrawer : public mono::IDrawable
    {
    public:

        WorldBoundsDrawer(const mono::TransformSystem* transform_system, const WorldBoundsSystem* world_system, PolygonDrawLayer draw_layer);

        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

        void BuildBuffers(const WorldBoundsComponent& world_bounds_component) const;

        const mono::TransformSystem* m_transform_system;
        const WorldBoundsSystem* m_world_system;
        const PolygonDrawLayer m_draw_layer;

        struct InternalRenderData
        {
            uint32_t timestamp;
            std::unique_ptr<mono::IRenderBuffer> vertices;
            std::unique_ptr<mono::IRenderBuffer> colors;
            std::unique_ptr<mono::IRenderBuffer> uvs;
            std::unique_ptr<mono::IElementBuffer> indices;
        };
        mutable std::unordered_map<uint32_t, InternalRenderData> m_id_to_buffers;
    };
}
