
#pragma once

#include "MonoFwd.h"
#include "WorldBoundsTypes.h"
#include "Rendering/Color.h"
#include "IGameSystem.h"
#include "Math/Vector.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Triangulate.h"

#include <vector>
#include <string>

namespace game
{
    struct WorldBoundsComponent
    {
        uint32_t id;
        uint32_t timestamp;
        mono::ITexturePtr texture;
        mono::Color::RGBA color_tint;
        PolygonDrawLayer draw_layer;
        std::vector<math::Vector> outline;
        mono::TriangulatedPoints triangulated_points;
    };

    class WorldBoundsSystem : public mono::IGameSystem
    {
    public:

        WorldBoundsSystem(mono::TransformSystem* transform_system);

        void AllocateTexturedPolygon(uint32_t id);
        void ReleaseTexturedPolygon(uint32_t id);
        void AddPolygon(
            uint32_t id,
            const std::vector<math::Vector>& vertices,
            const std::string& texture_file,
            const mono::Color::RGBA& color,
            PolygonDrawLayer draw_layer);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        template <typename T>
        inline void ForEachComponent(T&& func) const
        {
            for(uint32_t index = 0; index < m_components.size(); ++index)
                func(m_components[index]);
        }

        mono::TransformSystem* m_transform_system;
        std::vector<WorldBoundsComponent> m_components;
    };
}
