
#pragma once

#include "Math/Vector.h"
#include "Zone/EntityBase.h"
#include "Rendering/RenderPtrFwd.h"

#include "WorldFile.h"

#include <vector>

namespace editor
{
    class PolygonEntity : public mono::EntityBase
    {
    public:

        PolygonEntity();
        PolygonEntity(const math::Vector& position, const std::vector<math::Vector>& world_points);

        void AddVertex(const math::Vector& local_point);
        void SetVertex(const math::Vector& world_point, size_t index);

        // Local vertices
        const std::vector<math::Vector>& GetVertices() const;
        
        void SetSelected(bool selected);
        bool IsSelected() const;
        
        void SetTexture(const char* texture);
        const char* GetTextureName() const;
        
        virtual math::Quad BoundingBox() const;

    private:

        virtual void Draw(mono::IRenderer& renderer) const;
        virtual void Update(const mono::UpdateContext& update_context);

        math::Quad LocalBoundingBox() const;

        void RecalculateTextureCoordinates();

        bool m_selected;
        mono::ITexturePtr m_texture;

        std::vector<math::Vector> m_points;
        std::vector<math::Vector> m_textureCoordinates;
        char m_texture_name[world::PolygonTextureNameMaxLength];
    };

    void DrawPolygon(mono::IRenderer& renderer,
                     const mono::ITexturePtr& texture,
                     const std::vector<math::Vector>& points,
                     const std::vector<math::Vector>& texture_coords);
}
