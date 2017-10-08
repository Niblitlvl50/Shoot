
#include "NavMeshVisualizer.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFwd.h"

#include "NavMesh.h"

using namespace game;

NavMeshVisualizer::NavMeshVisualizer(
    const std::vector<math::Vector>& nav_mesh, const std::vector<NavmeshNode>& nodes)
    : m_nav_mesh(nav_mesh),
      m_nav_mesh_nodes(nodes)
{
    for(auto& node : nodes)
    {
        for(int neighbour : node.neighbours_index)
        {
            if(neighbour != -1)
            {
                m_edges.push_back(nav_mesh[node.data_index]);
                m_edges.push_back(nav_mesh[neighbour]);
            }
        }
    }
}

void NavMeshVisualizer::doDraw(mono::IRenderer& renderer) const
{
    constexpr mono::Color::RGBA edge_color(0.0f, 1.0f, 0.0f);
    renderer.DrawLines(m_edges, edge_color, 1.0f);

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
    renderer.DrawPoints(m_nav_mesh, color, 2.0f);
}

math::Quad NavMeshVisualizer::BoundingBox() const
{
    return math::Quad(-math::INF, -math::INF, math::INF, math::INF);
}
