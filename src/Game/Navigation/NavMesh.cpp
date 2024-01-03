
#include "NavMesh.h"
#include "NavmeshFactory.h"
#include "Math/MathFunctions.h"
#include "WorldFile.h"
#include "Util/Algorithm.h"

#include <set>
#include <unordered_set>
#include <unordered_map>
#include <cmath>

std::vector<math::Vector> game::GenerateMeshPoints(const math::Vector start, const math::Vector& end, float density)
{
    const math::Vector delta = end - start;
    const math::Vector multiplier = {
        delta.x > 0.0f ? 1.0f : -1.0f, 
        delta.y > 0.0f ? 1.0f : -1.0f, };

    const int n_width_points = std::fabs(delta.x) / density + 1;
    const int n_height_points = std::fabs(delta.y) / density + 1;

    std::vector<math::Vector> nav_mesh;
    nav_mesh.reserve(n_width_points * n_height_points);

    for(int width_index = 0; width_index < n_width_points; ++width_index)
    {
        for(int height_index = 0; height_index < n_height_points; ++height_index)
        {
            const math::Vector& point = math::Vector(width_index, height_index) * density * multiplier + start;
            nav_mesh.emplace_back(point);
        }
    }

    return nav_mesh;
}

std::vector<game::NavmeshNode> game::GenerateMeshNodes(
    const std::vector<math::Vector>& points,  float connection_distance, const NavmeshConnectionFilter& filter_function)
{
    const float connection_distance_squared = connection_distance * connection_distance;

    std::vector<game::NavmeshNode> nodes;
    nodes.reserve(points.size());

    for(uint32_t index = 0; index < points.size(); ++index)
    {
        const math::Vector& point = points[index];

        NavmeshNode node;
        node.data_index = index;
        std::fill(std::begin(node.neighbours_index), std::end(node.neighbours_index), -1);

        uint32_t neighbour_count = 0;

        for(uint32_t inner_index = 0; inner_index < points.size() && neighbour_count < std::size(node.neighbours_index); ++inner_index)
        {
            const math::Vector& inner_point = points[inner_index];
            const float distance_squared = math::DistanceBetweenSquared(point, inner_point);
            if(distance_squared == 0.0f)
                continue;

            if(distance_squared > connection_distance_squared)
                continue;

            const bool discard_connection = filter_function(point, inner_point);
            if(!discard_connection)
            {
                node.neighbours_index[neighbour_count] = inner_index;
                neighbour_count++;
            }
        }

        nodes.push_back(node);
    }

    return nodes;
}

float Heuristics(const game::NavmeshContext& context, int from, int to)
{
    return math::DistanceBetweenSquared(context.points[from], context.points[to]);
}

game::NavigationResult game::AStar(const game::NavmeshContext& context, int start_index, int end_index)
{
    NavigationResult result;
    result.nodes_evaluated = 0;
    result.result = AStarResult::FAILED;

    if(context.nodes.empty() || context.points.empty())
        return result;

    if(start_index == -1 || end_index == -1)
        return result;

    if(start_index == end_index)
    {
        result.result = AStarResult::SUCCESS;
        return result;
    }

    std::unordered_map<int, int> came_from;

    std::unordered_set<int> closed_set;
    std::unordered_set<int> open_set;

    std::vector<float> g_score(context.nodes.size(), math::INF);
    std::vector<float> f_score(context.nodes.size(), math::INF);

    g_score[start_index] = 0;
    f_score[start_index] = Heuristics(context, start_index, end_index);
    
    const auto find_lowest_f = [&f_score](const std::unordered_set<int>& open_set) {
        
        float lowest_f = math::INF;
        int lowest_index = 0;
        
        for(int open_node : open_set)
        {
            const float f_value = f_score[open_node];
            if(f_value < lowest_f)
            {
                lowest_f = f_value;
                lowest_index = open_node;
            }
        }

        return lowest_index;
    };

    open_set.insert(start_index);

    while(!open_set.empty())
    {
        const int current_index = find_lowest_f(open_set);

        open_set.erase(current_index);
        closed_set.insert(current_index);

        // We are at the goal!
        if(current_index == end_index)
            break;

        const NavmeshNode& node = context.nodes[current_index];

        for(int neighbour_index : node.neighbours_index)
        {
            if(neighbour_index == -1)
                continue;

            // Node is already evaluated
            if(closed_set.count(neighbour_index) == 1)
                continue;

            if(open_set.count(neighbour_index) == 0)
                open_set.insert(neighbour_index);

            const float tentative_g_score =
                g_score[current_index] + Heuristics(context, current_index, neighbour_index);

            // Not better path
            if(tentative_g_score >= g_score[neighbour_index])
                continue;

            came_from[neighbour_index] = current_index;
            g_score[neighbour_index] = tentative_g_score;
            f_score[neighbour_index] = tentative_g_score + Heuristics(context, neighbour_index, end_index);
        }
    }

    if(came_from.empty())
    {
        result.result = AStarResult::NO_PATH;
        return result;
    }
    
    int current = end_index;

    // Unravel the path
    while(current != start_index)
    {
        result.path_indices.push_back(current);
        const auto it = came_from.find(current);
        if(it == came_from.end())
            break;

        current = it->second;
    }

    if(current == start_index)
        result.path_indices.push_back(start_index);
    
    std::reverse(result.path_indices.begin(), result.path_indices.end());

    result.result = AStarResult::SUCCESS;
    result.nodes_evaluated = closed_set.size();

    return result;
}

game::NavigationResult game::AStar(const game::NavmeshContext& context, const math::Vector& start, const math::Vector& end)
{
    const int start_index = game::FindClosestIndex(context, start);
    const int end_index = game::FindClosestIndex(context, end);

    return AStar(context, start_index, end_index);
}

std::vector<math::Vector> game::PathToPoints(const game::NavmeshContext& context, const std::vector<int>& path)
{
    std::vector<math::Vector> points;
    points.reserve(path.size());

    for(int index : path)
        points.push_back(context.points[index]);

    return points;
}

int game::FindClosestIndex(const game::NavmeshContext& context, const math::Vector& point)
{
    int closest_index = -1;
    float closest_distance = math::INF;

    for(uint32_t index = 0, end = context.points.size(); index < end; ++index)
    {
        const math::Vector& node_point = context.points[index];
        const float distance = math::DistanceBetweenSquared(point, node_point);
        if(distance < closest_distance)
        {
            closest_distance = distance;
            closest_index = index;
        }
    }

    return closest_index;
}
