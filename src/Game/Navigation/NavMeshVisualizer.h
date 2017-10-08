
#pragma once

#include "Rendering/IDrawable.h"
#include <vector>

namespace game
{
    class NavMeshVisualizer : public mono::IDrawable
    {
    public:

        NavMeshVisualizer(const std::vector<math::Vector>& nav_mesh, const std::vector<struct NavmeshNode>& nodes);

        virtual void doDraw(mono::IRenderer& renderer) const;
        virtual math::Quad BoundingBox() const;

        const std::vector<math::Vector>& m_nav_mesh;
        const std::vector<NavmeshNode>& m_nav_mesh_nodes;

        std::vector<math::Vector> m_edges;
    };
}
