
#include "NavMesh.h"
#include "Math/MathFunctions.h"

std::vector<math::Vector> game::GenerateMeshPoints(const math::Vector start, float width, float height, float density)
{
    const int n_width_points = width / density;
    const int n_height_points = height / density;

    std::vector<math::Vector> nav_mesh;
    nav_mesh.reserve(n_width_points * n_height_points);

    for(int width_index = 0; width_index < n_width_points; ++width_index)
    {
        for(int height_index = 0; height_index < n_height_points; ++height_index)
        {
            const math::Vector& point = math::Vector(width_index * density, height_index * density) + start;
            nav_mesh.emplace_back(point);
        }
    }

    return nav_mesh;
}

void game::FilterNavMesh(std::vector<math::Vector>& nav_mesh, const std::vector<world::PolygonData>& static_polygons)
{
    const auto func = [&static_polygons](const math::Vector& point) {
        for(const world::PolygonData& polygon : static_polygons)
        {
            if(math::PointInsidePolygon(point, polygon.vertices))
                return true;
        }

        return false;
    };

    auto it = std::remove_if(nav_mesh.begin(), nav_mesh.end(), func);
    nav_mesh.erase(it, nav_mesh.end());
}

std::vector<game::NavmeshNode> game::GenerateMeshNodes(
    const std::vector<math::Vector>& points, const std::vector<world::PolygonData>& static_polygons, float connection_distance)
{
    std::vector<game::NavmeshNode> nodes;
    nodes.reserve(points.size());

    for(size_t index = 0; index < points.size(); ++index)
    {
        const math::Vector& point = points[index];

        NavmeshNode node;
        node.data_index = index;
        node.neighbours_index.fill(-1);

        size_t neighbour_count = 0;

        for(size_t inner_index = 0; inner_index < points.size() && neighbour_count < node.neighbours_index.size(); ++inner_index)
        {
            const math::Vector& inner_point = points[inner_index];
            const float distance = math::Length(point - inner_point);
            if(distance == 0.0f)
                continue;

            if(distance > connection_distance)
                continue;

            node.neighbours_index[neighbour_count] = inner_index;
            neighbour_count++;
        }

        nodes.push_back(node);
    }

    return nodes;
}
