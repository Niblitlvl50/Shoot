
#include "Polygon.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "Rendering/Texture/ITextureFactory.h"
#include "Math/Quad.h"
#include "Math/Matrix.h"
#include "Math/MathFwd.h"
#include "Math/MathFunctions.h"

#include "WorldFile.h"
#include <cassert>
#include <cstring>

namespace
{
    constexpr mono::Color::RGBA line_color(0.0f, 0.0f, 0.0f);
    constexpr mono::Color::RGBA point_color(0.0f, 1.0f, 0.7f);
}

using namespace editor;

PolygonEntity::PolygonEntity()
    : m_selected(false),
      m_texture_name("res/textures/placeholder.png")
{ }

PolygonEntity::PolygonEntity(const math::Vector& position, const std::vector<math::Vector>& world_points)
    : PolygonEntity()
{
    m_position = position;
    
    math::Matrix world_to_local = Transformation();
    math::Inverse(world_to_local);
    
    m_points.reserve(world_points.size());

    for(const math::Vector& world_point : world_points)
        m_points.push_back(math::Transform(world_to_local, world_point));

    RecalculateTextureCoordinates();
    m_pivot_point = math::CentroidOfPolygon(m_points);
}

void PolygonEntity::Draw(mono::IRenderer& renderer) const
{
    if(!m_texture)
        return;
    
    constexpr mono::Color::RGBA selected_color(0.0f, 1.0f, 0.0f);
    if(m_selected)
        renderer.DrawClosedPolyline(m_points, selected_color, 6.0f);

    DrawPolygon(renderer, m_texture, m_points, m_textureCoordinates);
    renderer.DrawPoints({ m_pivot_point }, point_color, 4.0f);
}

void PolygonEntity::Update(const mono::UpdateContext& update_context)
{
    if(!m_texture)
        m_texture = mono::GetTextureFactory()->CreateTexture(m_texture_name);
}

math::Quad PolygonEntity::BoundingBox() const
{
    const math::Matrix& local_to_world = Transformation();

    math::Quad bb(math::INF, math::INF, -math::INF, -math::INF);
    for(auto& point : m_points)
        bb |= math::Transform(local_to_world, point);

    return bb;
}

void PolygonEntity::AddVertex(const math::Vector& local_point)
{
    m_points.push_back(local_point);
    RecalculateTextureCoordinates();
    m_pivot_point = math::CentroidOfPolygon(m_points);
}

void PolygonEntity::SetVertex(const math::Vector& world_point, size_t index)
{
    math::Matrix world_to_local = Transformation();
    math::Inverse(world_to_local);

    m_points[index] = math::Transform(world_to_local, world_point);
    RecalculateTextureCoordinates();
}

const std::vector<math::Vector>& PolygonEntity::GetVertices() const
{
    return m_points;
}

void PolygonEntity::SetSelected(bool selected)
{
    m_selected = selected;
}

bool PolygonEntity::IsSelected() const
{
    return m_selected;
}

void PolygonEntity::SetTexture(const char* texture)
{
    const std::size_t length = std::strlen(texture);
    assert(length <= world::PolygonTextureNameMaxLength);

    std::memset(m_texture_name, 0, world::PolygonTextureNameMaxLength);
    std::memcpy(m_texture_name, texture, length);

    m_texture = nullptr;
}

const char* PolygonEntity::GetTextureName() const
{
    return m_texture_name;
}

math::Quad PolygonEntity::LocalBoundingBox() const
{
    math::Quad bb(math::INF, math::INF, -math::INF, -math::INF);

    for(auto& point : m_points)
        bb |= point;

    return bb;
}

void PolygonEntity::RecalculateTextureCoordinates()
{
    m_textureCoordinates.clear();

    const math::Quad& bb = LocalBoundingBox();
    const math::Vector& repeate = (bb.mB - bb.mA) / 2.0f;

    for(const math::Vector& point : m_points)
        m_textureCoordinates.push_back(math::MapVectorInQuad(point, bb) * repeate);
}

void editor::DrawPolygon(mono::IRenderer& renderer,
                         const mono::ITexturePtr& texture,
                         const std::vector<math::Vector>& points,
                         const std::vector<math::Vector>& texture_coords)
{
    std::vector<unsigned short> indices;
    for(size_t index = 0; index < points.size(); ++index)
        indices.push_back(index);

    renderer.DrawGeometry(points, texture_coords, indices, texture);
    renderer.DrawClosedPolyline(points, line_color, 2.0f);
    renderer.DrawPoints(points, point_color, 4.0f);
}

