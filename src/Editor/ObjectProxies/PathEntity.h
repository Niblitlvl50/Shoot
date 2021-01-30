
#pragma once

#include "IUpdatable.h"
#include "Rendering/IDrawable.h"
#include "Math/Vector.h"
#include "Paths/Bezier.h"
#include <vector>
#include <string>

#include "Rendering/RenderBuffer/BufferFactory.h"

#include "par_streamlines/par_streamlines.h"

namespace editor
{
    class PathEntity : public mono::IUpdatable, public mono::IDrawable
    {
    public:

        PathEntity(const std::string& name);
        PathEntity(const std::string& name, const std::vector<math::Vector>& local_points);

        ~PathEntity();

        uint32_t Id() const;

        math::Matrix Transformation() const;

        void SetPosition(const math::Vector& position);
        const math::Vector& Position() const;

        void SetRotation(float rotation);
        float Rotation() const;

        void SetSelected(bool selected);
        void SetName(const char* new_name);
        const std::string& GetName() const;

        void SetVertex(const math::Vector& world_point, size_t index);
        const std::vector<math::Vector>& GetPoints() const;

        virtual void Draw(mono::IRenderer& renderer) const;
        virtual void Update(const mono::UpdateContext& update_context);
        virtual math::Quad BoundingBox() const;

        void UpdateMesh();

    private:
        std::string m_name;
        std::vector<math::Vector> m_points;
        bool m_selected;

        math::Vector m_position;
        float m_rotation;

        mono::Curve m_curve;

        parsl_context* m_ctx;
        uint32_t m_num_triangles;

        std::unique_ptr<mono::IRenderBuffer> m_position_buffer;
        std::unique_ptr<mono::IRenderBuffer> m_color_buffer;
        std::unique_ptr<mono::IElementBuffer> m_index_buffer;
    };

    void DrawPath(mono::IRenderer& renderer, const std::vector<math::Vector>& points);
}
