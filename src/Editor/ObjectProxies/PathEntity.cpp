
#define PAR_STREAMLINES_IMPLEMENTATION

#include "PathEntity.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFunctions.h"


#include <cmath>

using namespace editor;

namespace
{
    std::vector<math::Vector> GenerateArrows(const std::vector<math::Vector>& points)
    {
        std::vector<math::Vector> vertices;
        vertices.reserve(points.size() * 4);

        for(size_t index = 1; index < points.size(); ++index)
        {
            const math::Vector& first = points[index -1];
            const math::Vector& second = points[index];

            const float angle = math::AngleBetweenPoints(first, second);

            const float x1 = std::sin(angle + math::PI() + math::PI_4()) + second.x;
            const float y1 = -std::cos(angle + math::PI() + math::PI_4()) + second.y;

            const float x2 = std::sin(angle - math::PI_4()) + second.x;
            const float y2 = -std::cos(angle - math::PI_4()) + second.y;

            vertices.emplace_back(x1, y1);
            vertices.emplace_back(second);
            vertices.emplace_back(second);
            vertices.emplace_back(x2, y2);
        }

        return vertices;
    }

    constexpr mono::Color::RGBA line_color(0.0f, 0.0f, 0.0f, 0.4f);
    constexpr mono::Color::RGBA arrow_color(0.0f, 1.0f, 0.7f, 0.4f);
    constexpr mono::Color::RGBA selected_color(0.0f, 1.0f, 0.0f);
}

PathEntity::PathEntity(const std::string& name)
    : m_name(name)
    , m_selected(false)
    , m_num_triangles(0)
{
    m_curve = mono::MakeCurve(mono::CurveType::CUBIC);

/*
    m_curve.points = {
        {1, 8},
        {9.5, 1},
        {18, 8},
        {20, 5},
        {18, 3},
    };
*/

    m_curve.points = {
        { 1, 8 },
        { 4, 10 },
        { 6.5, 1 },
        { 9.5, 8 },
        { 15, 15 },
        { 18, 8 },
        { 18, 8 },
        { 18, 8 },
    };

    parsl_config config;
    config.thickness = 0.1;
    config.curves_max_flatness = 0.25;
    m_ctx = parsl_create_context(config);
}

PathEntity::PathEntity(const std::string& name, const std::vector<math::Vector>& local_points)
    : PathEntity(name)
{
    m_points = local_points;
    m_pivot_point = math::CentroidOfPolygon(m_points);

    UpdateMesh();
}

PathEntity::~PathEntity()
{
    parsl_destroy_context(m_ctx);
}

void PathEntity::EntityDraw(mono::IRenderer& renderer) const
{
    std::vector<math::Vector> curve_points;
    std::vector<math::Vector> control_points;

    for(size_t index = 0; index < m_curve.points.size(); ++index)
    {
        if(mono::IsControlPoint(m_curve, index))
            control_points.push_back(m_curve.points[index]);
        else
            curve_points.push_back(m_curve.points[index]);
    }

    renderer.DrawPoints( control_points, mono::Color::RED, 4.0f);
    renderer.DrawPoints( curve_points, mono::Color::BLACK, 2.0f);


    renderer.DrawPolyline(m_curve.points, selected_color, 1.0f);
    //DrawPath(renderer, m_curve.points);

/*
    if(m_selected)
        renderer.DrawPolyline(m_points, selected_color, 4.0f);

    DrawPath(renderer, m_points);
    renderer.DrawPoints( { m_pivot_point }, arrow_color, 4.0f);
*/
    if(m_num_triangles != 0)
        renderer.DrawTrianges(m_position_buffer.get(), m_color_buffer.get(), m_index_buffer.get(), m_num_triangles * 3);
}

void PathEntity::EntityUpdate(const mono::UpdateContext& update_context)
{ }

math::Quad PathEntity::BoundingBox() const
{
    const math::Matrix& local_to_world = Transformation();
    math::Quad bb(math::INF, math::INF, -math::INF, -math::INF);
    for(auto& point : m_points)
        bb |= math::Transform(local_to_world, point);

    return bb;
}

void PathEntity::SetSelected(bool selected)
{
    m_selected = selected;
}

void PathEntity::SetVertex(const math::Vector& world_point, size_t index)
{
    const math::Matrix& world_to_local = math::Inverse(Transformation());
    //m_points[index] = math::Transform(world_to_local, world_point);
    m_curve.points[index] = math::Transform(world_to_local, world_point);

    UpdateMesh();
}

const std::vector<math::Vector>& PathEntity::GetPoints() const
{
    //return m_points;
    return m_curve.points;
}

void PathEntity::SetName(const char* new_name)
{
    m_name = new_name;
}

const std::string& PathEntity::GetName() const
{
    return m_name;
}

void editor::DrawPath(mono::IRenderer& renderer, const std::vector<math::Vector>& points)
{
    renderer.DrawPolyline(points, line_color, 2.0f);

    const std::vector<math::Vector>& arrow_vertices = GenerateArrows(points);
    renderer.DrawLines(arrow_vertices, arrow_color, 2.0f);
}

void PathEntity::UpdateMesh()
{
    using namespace mono;

    //const uint32_t num_points = m_points.size();
    const uint32_t num_points = m_curve.points.size();

    uint16_t spine_length[] = { (uint16_t)num_points };

    parsl_spine_list spine_list;
    spine_list.num_vertices = num_points;
    spine_list.num_spines = 1;
    spine_list.vertices = (parsl_position*)m_curve.points.data();
    spine_list.spine_lengths = spine_length;
    spine_list.closed = true;

    //parsl_mesh* generated_mesh = parsl_mesh_from_lines(m_ctx, spine_list);
    //parsl_mesh* generated_mesh = parsl_mesh_from_curves_quadratic(m_ctx, spine_list);
    parsl_mesh* generated_mesh = parsl_mesh_from_curves_cubic(m_ctx, spine_list);

    m_num_triangles = generated_mesh->num_triangles;

    const uint32_t position_buffer_data_size = generated_mesh->num_vertices * 2;
    if(!m_position_buffer || m_position_buffer->Size() < position_buffer_data_size)
    {
        m_position_buffer = CreateRenderBuffer(BufferType::DYNAMIC, BufferData::FLOAT, 2, position_buffer_data_size);
    }
    m_position_buffer->UpdateData(generated_mesh->positions, 0, position_buffer_data_size);

    const uint32_t color_buffer_data_size = position_buffer_data_size * 2;
    if(!m_color_buffer || m_color_buffer->Size() < color_buffer_data_size)
    {
        m_color_buffer = CreateRenderBuffer(BufferType::DYNAMIC, BufferData::FLOAT, 4, color_buffer_data_size);
    }
    const std::vector<mono::Color::RGBA> colors(generated_mesh->num_vertices, mono::Color::RED);
    m_color_buffer->UpdateData(colors.data(), 0, color_buffer_data_size);

    const uint32_t index_buffer_data_size = generated_mesh->num_triangles * 3;
    if(!m_index_buffer || m_index_buffer->Size() < index_buffer_data_size)
    {
        m_index_buffer = CreateElementBuffer(BufferType::DYNAMIC, BufferData::INT, index_buffer_data_size);
    }
    m_index_buffer->UpdateData(generated_mesh->triangle_indices, 0, index_buffer_data_size);

    mono::ClearRenderBuffer();
}
