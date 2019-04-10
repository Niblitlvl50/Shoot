
#include "PolygonProxy.h"
#include "IObjectVisitor.h"
#include "Grabber.h"
#include "SnapPoint.h"
#include "UI/UIContext.h"
#include "Objects/Polygon.h"
#include "Resources.h"
#include "Math/MathFunctions.h"
#include "Math/Matrix.h"
#include "Math/Quad.h"

#include "ImGuiImpl/ImGuiImpl.h"
#include "ObjectAttribute.h"
#include "Component.h"

using namespace editor;

PolygonProxy::PolygonProxy(const std::shared_ptr<PolygonEntity>& polygon)
    : m_polygon(polygon)
{ }

PolygonProxy::~PolygonProxy()
{ }

const char* PolygonProxy::Name() const
{
    return "polygonobject";
}

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

bool PolygonProxy::Intersects(const math::Vector& world_point) const
{
    const math::Quad& bb = m_polygon->BoundingBox();
    const bool inside_quad = math::PointInsideQuad(world_point, bb);
    if(inside_quad)
    {
        math::Matrix world_to_local = m_polygon->Transformation();
        math::Inverse(world_to_local);

        const math::Vector& local_point = math::Transform(world_to_local, world_point);
        return math::PointInsidePolygon(local_point, m_polygon->GetVertices());
    }

    return false;
}

std::vector<Grabber> PolygonProxy::GetGrabbers() const
{
    using namespace std::placeholders;

    const math::Matrix& local_to_world = m_polygon->Transformation();
    const auto& local_vertices = m_polygon->GetVertices();

    std::vector<Grabber> grabbers;
    grabbers.reserve(local_vertices.size());

    for(size_t index = 0; index < local_vertices.size(); ++index)
    {
        Grabber grabber;
        grabber.position = math::Transform(local_to_world, local_vertices[index]);
        grabber.callback = std::bind(&PolygonEntity::SetVertex, m_polygon, _1, index);
        grabbers.push_back(grabber);
    }

    return grabbers;
}

std::vector<SnapPoint> PolygonProxy::GetSnappers() const
{
    return std::vector<SnapPoint>();
}

void PolygonProxy::UpdateUIContext(UIContext& context)
{
    const math::Vector& position = m_polygon->Position();
    const float rotation = m_polygon->Rotation();
    int texture_index = FindTextureIndex(m_polygon->GetTextureName());

    ImGui::Value("X", position.x);
    ImGui::SameLine();
    ImGui::Value("Y", position.y);
    ImGui::Value("Rotation", rotation);

    if(ImGui::Combo("Texture", &texture_index, avalible_textures, mono::arraysize(avalible_textures)))
        m_polygon->SetTexture(avalible_textures[texture_index]);
}

const std::vector<Component>& PolygonProxy::GetComponents() const
{
    return m_components;
}

std::vector<Component>& PolygonProxy::GetComponents()
{
    return m_components;
}

std::unique_ptr<editor::IObjectProxy> PolygonProxy::Clone() const
{
    return nullptr;
}

void PolygonProxy::Visit(IObjectVisitor& visitor)
{
    visitor.Accept(this);
}
