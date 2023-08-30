
#include "NavMeshVisualizer.h"
#include "NavigationSystem.h"

#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/MathFwd.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/MouseEvent.h"

#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "NavMesh.h"
#include "Debug/GameDebug.h"

#include "System/System.h"

using namespace game;

NavmeshVisualizer::NavmeshVisualizer(const NavigationSystem* navigation_system, mono::EventHandler& event_handler)
    : m_navigation_system(navigation_system),
      m_event_handler(event_handler)
{
    const NavmeshContext* navmesh_context = m_navigation_system->GetNavmeshContext();
    if(navmesh_context)
    {
        for(auto& node : navmesh_context->nodes)
        {
            for(int neighbour : node.neighbours_index)
            {
                if(neighbour != -1)
                {
                    m_edges.push_back(navmesh_context->points[node.data_index]);
                    m_edges.push_back(navmesh_context->points[neighbour]);
                }
            }
        }
    }

    using namespace std::placeholders;

    const event::MouseUpEventFunc mouse_up_func = std::bind(&NavmeshVisualizer::OnMouseUp, this, _1);
    m_mouse_up_token = m_event_handler.AddListener(mouse_up_func);

    const event::MouseMotionEventFunc mouse_motion_func = std::bind(&NavmeshVisualizer::OnMouseMove, this, _1);
    m_mouse_motion_token = m_event_handler.AddListener(mouse_motion_func);
}

NavmeshVisualizer::~NavmeshVisualizer()
{
    m_event_handler.RemoveListener(m_mouse_up_token);
    m_event_handler.RemoveListener(m_mouse_motion_token);
}

void NavmeshVisualizer::Draw(mono::IRenderer& renderer) const
{
    if(!game::g_draw_navmesh)
        return;

    if(game::g_draw_navmesh_subcomponents & game::NavigationDebugComponents::DRAW_NAVMESH)
        DrawNavmesh(renderer);
    
    if(game::g_draw_navmesh_subcomponents & game::NavigationDebugComponents::DRAW_RECENT_PATHS)
        DrawPaths(renderer);

    const int n_pathfinds = m_navigation_system->GetNumFindPath();
    if(n_pathfinds > 0)
        System::Log("[%u] PathFinds: %d", renderer.GetTimestamp(), n_pathfinds);

    //DrawInteractivePath(renderer);
}

math::Quad NavmeshVisualizer::BoundingBox() const
{
    return math::InfQuad;
}

void NavmeshVisualizer::DrawNavmesh(mono::IRenderer& renderer) const
{
    const NavmeshContext* navmesh_context = m_navigation_system->GetNavmeshContext();
    if(!navmesh_context)
        return;

    if(navmesh_context->nodes.empty() || navmesh_context->points.empty())
        return;

    constexpr mono::Color::RGBA edge_color(0.0f, 1.0f, 0.0f, 0.2f);
    renderer.DrawLines(m_edges, edge_color, 1.0f);
    renderer.DrawPoints(navmesh_context->points, mono::Color::MAGENTA, 2.0f);
}

void NavmeshVisualizer::DrawPaths(mono::IRenderer& renderer) const
{
    const uint32_t timestamp = renderer.GetTimestamp();

    const std::vector<RecentPath>& recent_paths = m_navigation_system->GetRecentPaths();
    for(const RecentPath& path : recent_paths)
    {
        const uint32_t delta_time = timestamp - path.timestamp;
        const float alpha = math::Scale01(delta_time, 5000u, 0u);
        if(alpha < 0.0f)
            continue;

        renderer.DrawPolyline(path.points, mono::Color::MakeWithAlpha(mono::Color::MAGENTA, alpha), 2.0f);
        renderer.DrawPoints(path.points, mono::Color::MakeWithAlpha(mono::Color::CYAN, alpha), 4.0f);
    }
}

void NavmeshVisualizer::DrawInteractivePath(mono::IRenderer& renderer) const
{
    const NavmeshContext* navmesh_context = m_navigation_system->GetNavmeshContext();
    if(!navmesh_context)
        return;

    if(navmesh_context->nodes.empty() || navmesh_context->points.empty())
        return;

    if(m_path)
    {
        const std::vector<math::Vector>& path_points = m_path->GetPathPoints();
        renderer.DrawPolyline(path_points, mono::Color::MAGENTA, 2.0f);

        const math::Vector point_on_line = m_path->GetPositionByLength(m_at_length);
        renderer.DrawPoints({ point_on_line }, mono::Color::CYAN, 4.0f);
    }

    std::vector<math::Vector> start_node_points;
    const game::NavmeshNode& start_node = navmesh_context->nodes[m_start];

    start_node_points.push_back(navmesh_context->points[start_node.data_index]);

    for(int point_index : start_node.neighbours_index)
    {
        if(point_index != -1)
            start_node_points.push_back(navmesh_context->points[point_index]);
    }

    constexpr mono::Color::RGBA selected_color(1.0f, 1.0f, 0.0f);
    renderer.DrawPoints(start_node_points, selected_color, 4.0f);
}

mono::EventResult NavmeshVisualizer::OnMouseUp(const event::MouseUpEvent& event)
{
    const NavmeshContext* navmesh_context = m_navigation_system->GetNavmeshContext();
    if(game::g_draw_navmesh && navmesh_context)
    {
        const math::Vector position(event.world_x, event.world_y);

        if(event.key == MouseButton::LEFT)
            m_start = game::FindClosestIndex(*navmesh_context, position);
        else
            m_end = game::FindClosestIndex(*navmesh_context, position);

        const std::vector<int>& nav_path = game::AStar(*navmesh_context, m_start, m_end);
        const std::vector<math::Vector>& nav_points = game::PathToPoints(*navmesh_context, nav_path);
        m_path = mono::CreatePath(nav_points);
    }

    return mono::EventResult::PASS_ON;
}

mono::EventResult NavmeshVisualizer::OnMouseMove(const event::MouseMotionEvent& event)
{
    if(game::g_draw_navmesh && m_path)
        m_at_length = m_path->GetLengthFromPosition(math::Vector(event.world_x, event.world_y));

    return mono::EventResult::PASS_ON;
}
