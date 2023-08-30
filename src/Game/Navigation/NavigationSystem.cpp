
#include "NavigationSystem.h"
#include "Navigation/NavmeshFactory.h"
#include "Navigation/NavMesh.h"
#include "CollisionConfiguration.h"

#include "Physics/PhysicsSpace.h"
#include "Util/Algorithm.h"


namespace
{
    constexpr int NumRecentPaths = 10;
}

using namespace game;

NavigationSystem::NavigationSystem()
{
    m_current_path_index = 0;
    m_recent_paths.resize(NumRecentPaths);
    m_findpath_this_frame = 0;
}

const char* NavigationSystem::Name() const
{
    return "navigationsystem";
}

void NavigationSystem::Reset()
{
    m_navmesh.points.clear();
    m_navmesh.nodes.clear();
}

void NavigationSystem::Update(const mono::UpdateContext& update_context)
{
    m_timestamp = update_context.timestamp;
}

void NavigationSystem::Sync()
{
    m_findpath_this_frame = 0;
}

void NavigationSystem::SetupNavmesh(const math::Vector& start, const math::Vector& end, float density, mono::PhysicsSpace* physics_space)
{
    m_navmesh.points = game::GenerateMeshPoints(start, end, density);

    const auto remove_on_collision = [physics_space](const math::Vector& point) {
        const mono::QueryResult query_result = physics_space->QueryNearest(point, 0.0f, game::CollisionCategory::STATIC);
        return query_result.body != nullptr;
    };
    mono::remove_if(m_navmesh.points, remove_on_collision);

    const auto filter_connection_func = [physics_space](const math::Vector& first, const math::Vector& second){
        const mono::QueryResult query_result = physics_space->QueryFirst(first, second, game::CollisionCategory::STATIC);
        return query_result.body != nullptr;
    };

    m_navmesh.nodes = game::GenerateMeshNodes(m_navmesh.points, density * 1.5f, filter_connection_func);
}

const NavmeshContext* NavigationSystem::GetNavmeshContext() const
{
    return &m_navmesh;
}

const std::vector<math::Vector>& NavigationSystem::FindPath(const math::Vector& start_position, const math::Vector& end_position)
{
    m_findpath_this_frame++;

    static const std::vector<math::Vector> empty_path;

    const int start_index = game::FindClosestIndex(m_navmesh, start_position);
    const int end_index = game::FindClosestIndex(m_navmesh, end_position);

    if(start_index == end_index || start_index == -1 || end_index == -1)
        return empty_path;

    const std::vector<int>& nav_path = game::AStar(m_navmesh, start_index, end_index);
    if(nav_path.empty())
        return empty_path;

    RecentPath& recent_path = m_recent_paths[m_current_path_index % NumRecentPaths];
    recent_path.timestamp = m_timestamp;
    recent_path.points = game::PathToPoints(m_navmesh, nav_path);

    // Replace the first navmesh node position with current position, this creates a much better start and when updating a path.
    recent_path.points.front() = start_position;

    m_current_path_index++;

    return recent_path.points;
}

const std::vector<RecentPath>& NavigationSystem::GetRecentPaths() const
{
    return m_recent_paths;
}

int NavigationSystem::GetNumFindPath() const
{
    return m_findpath_this_frame;
}
