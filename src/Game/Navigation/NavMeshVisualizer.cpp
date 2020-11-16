
#include "NavMeshVisualizer.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFwd.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"

#include "NavMesh.h"
#include "GameDebug.h"

using namespace game;

NavmeshVisualizer::NavmeshVisualizer(const NavmeshContext& context, mono::EventHandler& event_handler)
    : m_navmesh_context(context),
      m_event_handler(event_handler)
{
    for(auto& node : m_navmesh_context.nodes)
    {
        for(int neighbour : node.neighbours_index)
        {
            if(neighbour != -1)
            {
                m_edges.push_back(m_navmesh_context.points[node.data_index]);
                m_edges.push_back(m_navmesh_context.points[neighbour]);
            }
        }
    }

    using namespace std::placeholders;
    
    const event::MouseUpEventFunc mouse_up_func = std::bind(&NavmeshVisualizer::OnMouseUp, this, _1);
    m_mouse_up_token = m_event_handler.AddListener(mouse_up_func);
}

NavmeshVisualizer::~NavmeshVisualizer()
{
    m_event_handler.RemoveListener(m_mouse_up_token);
}

void NavmeshVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_navmesh)
        return;

    if(m_navmesh_context.nodes.empty() || m_navmesh_context.points.empty())
        return;

    constexpr mono::Color::RGBA edge_color(0.0f, 1.0f, 0.0f, 0.2f);
    renderer.DrawLines(m_edges, edge_color, 1.0f);

    constexpr mono::Color::RGBA point_color(1.0f, 0.0f, 1.0f);
    renderer.DrawPoints(m_navmesh_context.points, point_color, 2.0f);

    constexpr mono::Color::RGBA color(1.0f, 0.0f, 1.0f);
    renderer.DrawPolyline(m_navigation_points, color, 2.0f);

    std::vector<math::Vector> start_node_points;
    const game::NavmeshNode& start_node = m_navmesh_context.nodes[m_start];

    start_node_points.push_back(m_navmesh_context.points[start_node.data_index]);

    for(int point_index : start_node.neighbours_index)
        start_node_points.push_back(m_navmesh_context.points[point_index]);

    constexpr mono::Color::RGBA selected_color(1.0f, 1.0f, 0.0f);
    renderer.DrawPoints(start_node_points, selected_color, 4.0f);
}

math::Quad NavmeshVisualizer::BoundingBox() const
{
    return math::InfQuad;
}

mono::EventResult NavmeshVisualizer::OnMouseUp(const event::MouseUpEvent& event)
{
    const math::Vector position(event.world_x, event.world_y);

    if(event.key == MouseButton::LEFT)
        m_start = game::FindClosestIndex(m_navmesh_context, position);
    else
        m_end = game::FindClosestIndex(m_navmesh_context, position);

    m_navigation_points.clear();
        
    for(int index : game::AStar(m_navmesh_context, m_start, m_end))
        m_navigation_points.push_back(m_navmesh_context.points[index]);

    return mono::EventResult::PASS_ON;
}
