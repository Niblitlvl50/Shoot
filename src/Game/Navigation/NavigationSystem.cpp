
#include "NavigationSystem.h"
#include "Navigation/NavmeshFactory.h"
#include "Physics/PhysicsSpace.h"

#include "CollisionConfiguration.h"
#include "Util/Algorithm.h"

using namespace game;

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
