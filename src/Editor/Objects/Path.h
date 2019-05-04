
#pragma once

#include "Entity/EntityBase.h"
#include "Math/Vector.h"
#include <vector>
#include <string>

namespace editor
{
    class PathEntity : public mono::EntityBase
    {
    public:

        PathEntity(const std::string& name);
        PathEntity(const std::string& name, const std::vector<math::Vector>& local_points);

        void SetSelected(bool selected);
        void SetName(const char* new_name);

        void SetVertex(const math::Vector& world_point, size_t index);

        virtual void Draw(mono::IRenderer& renderer) const;
        virtual void Update(const mono::UpdateContext& update_context);
        virtual math::Quad BoundingBox() const;

        std::string m_name;
        std::vector<math::Vector> m_points;
        bool m_selected;
    };

    void DrawPath(mono::IRenderer& renderer, const std::vector<math::Vector>& points);
}
