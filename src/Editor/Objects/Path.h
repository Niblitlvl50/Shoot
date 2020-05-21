
#pragma once

#include "Zone/EntityBase.h"
#include "Math/Vector.h"
#include "Paths/Bezier.h"
#include <vector>
#include <string>

#include "Rendering/BufferFactory.h"

#include "par_streamlines/par_streamlines.h"

namespace editor
{
    class PathEntity : public mono::EntityBase
    {
    public:

        PathEntity(const std::string& name);
        PathEntity(const std::string& name, const std::vector<math::Vector>& local_points);

        ~PathEntity();

        void SetSelected(bool selected);
        void SetName(const char* new_name);
        const std::string& GetName() const;

        void SetVertex(const math::Vector& world_point, size_t index);
        const std::vector<math::Vector>& GetPoints() const;

        virtual void EntityDraw(mono::IRenderer& renderer) const;
        virtual void EntityUpdate(const mono::UpdateContext& update_context);
        virtual math::Quad BoundingBox() const;

        void UpdateMesh();

    private:
        std::string m_name;
        std::vector<math::Vector> m_points;
        bool m_selected;

        mono::Curve m_curve;

        parsl_context* m_ctx;
        uint32_t m_num_triangles;

        std::unique_ptr<mono::IRenderBuffer> m_position_buffer;
        std::unique_ptr<mono::IRenderBuffer> m_color_buffer;
        std::unique_ptr<mono::IRenderBuffer> m_index_buffer;
    };

    void DrawPath(mono::IRenderer& renderer, const std::vector<math::Vector>& points);
}
