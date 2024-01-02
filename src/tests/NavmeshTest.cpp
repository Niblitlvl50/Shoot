
#include "Navigation/NavMesh.h"
#include "Navigation/NavmeshFactory.h"
#include "Util/Random.h"

#include "gtest/gtest.h"

#include <numeric>

class Navmesh : public testing::Test 
{
protected:
    virtual void SetUp()
    {
        const auto connection_filter = [](const math::Vector& first, const math::Vector& second) -> bool {
            return false;
        };

        m_min_navmesh = math::Vector(-35.0f, -25.0f);
        m_max_navmesh = math::Vector(35.0f, 25.0f);

        constexpr float density = 1.0f;

        m_context.points = game::GenerateMeshPoints(m_min_navmesh, m_max_navmesh, density);
        m_context.nodes = game::GenerateMeshNodes(m_context.points, density * 1.5f, connection_filter);
    }
    
    game::NavmeshContext m_context;
    math::Vector m_min_navmesh;
    math::Vector m_max_navmesh;
};

TEST_F(Navmesh, AStar)
{
    std::vector<int> nodes_evaluated;
    nodes_evaluated.reserve(100);

    for(int index = 0; index < 100; ++index)
    {
        const math::Vector start_position = {
            mono::Random(m_min_navmesh.x, m_max_navmesh.x),
            mono::Random(m_min_navmesh.y, m_max_navmesh.y)
        };

        const math::Vector end_position = {
            mono::Random(m_min_navmesh.x, m_max_navmesh.x),
            mono::Random(m_min_navmesh.y, m_max_navmesh.y)
        };

        const game::NavigationResult& nav_result = game::AStar(m_context, start_position, end_position);
        EXPECT_TRUE(nav_result.result == game::AStarResult::SUCCESS);

        nodes_evaluated.push_back(nav_result.nodes_evaluated);
    }

    const int total_nodes = std::accumulate(nodes_evaluated.begin(), nodes_evaluated.end(), 0);
    std::printf("Average number of nodes evaluated: %u\n", total_nodes / nodes_evaluated.size());
}
