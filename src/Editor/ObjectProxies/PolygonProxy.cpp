
#include "PolygonProxy.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "UIContext.h"
#include "Polygon.h"
#include "Textures.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

#include "ImGuiImpl/ImGuiImpl.h"

using namespace editor;

PolygonProxy::PolygonProxy(const std::shared_ptr<PolygonEntity>& polygon)
    : m_polygon(polygon)
{ }

unsigned int PolygonProxy::Id() const
{
    return m_polygon->Id();
}

mono::IEntityPtr PolygonProxy::Entity()
{
    return m_polygon;
}

void PolygonProxy::SetSelected(bool selected)
{
    m_polygon->SetSelected(selected);
}

bool PolygonProxy::Intersects(const math::Vector& position) const
{
    const math::Quad& bb = m_polygon->BoundingBox();
    const bool inside_quad = math::PointInsideQuad(position, bb);
    if(inside_quad)
    {
        math::Matrix transform = m_polygon->Transformation();
        math::Inverse(transform);
        const math::Vector& local_position = math::Transform(transform, position);
        return math::PointInsidePolygon(local_position, m_polygon->GetVertices());
    }

    return false;
}

std::vector<Grabber> PolygonProxy::GetGrabbers() const
{
    using namespace std::placeholders;

    const math::Matrix& transform = m_polygon->Transformation();
    const auto& vertices = m_polygon->GetVertices();

    std::vector<Grabber> grabbers;
    grabbers.reserve(vertices.size());

    for(size_t index = 0; index < vertices.size(); ++index)
    {
        Grabber grab;
        grab.position = math::Transform(transform, vertices[index]);
        grab.callback = std::bind(&PolygonEntity::SetVertex, m_polygon, _1, index);
        grabbers.push_back(grab);
    }

    return grabbers;
}

std::vector<SnapPoint> PolygonProxy::GetSnappers() const
{
    return std::vector<SnapPoint>();
}

void PolygonProxy::UpdateUIContext(UIContext& context) const
{
    const math::Vector& position = m_polygon->Position();
    const float rotation = m_polygon->Rotation();
    int texture_index = FindTextureIndex(m_polygon->GetTexture());

    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);
    ImGui::Value("Rotation", rotation);

    if(ImGui::Combo("Texture", &texture_index, avalible_textures, n_textures))
        m_polygon->SetTexture(avalible_textures[texture_index]);
}
