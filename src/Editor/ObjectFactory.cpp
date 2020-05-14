
#include "ObjectFactory.h"

#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"

#include "Objects/Path.h"
#include "Objects/Polygon.h"

#include "Math/Matrix.h"

using namespace editor;

ObjectFactory::ObjectFactory(Editor* editor)
    : m_editor(editor)
{
}

IObjectProxyPtr ObjectFactory::CreatePath(const std::string& name, const std::vector<math::Vector>& points) const
{
    auto path = std::make_unique<editor::PathEntity>(name, points);
    return std::make_unique<PathProxy>(std::move(path), m_editor);
}

IObjectProxyPtr ObjectFactory::CreatePolygon(
    const std::string& name,
    const math::Vector& position,
    const math::Vector& base_point,
    float rotation,
    const std::vector<math::Vector>& vertices,
    const std::string& texture_name) const
{
    auto polygon_entity = std::make_unique<editor::PolygonEntity>();
    polygon_entity->SetPosition(position);
    polygon_entity->SetBasePoint(base_point);
    polygon_entity->SetRotation(rotation);
    polygon_entity->SetTexture(texture_name.c_str());

    math::Matrix world_to_local = polygon_entity->Transformation();
    math::Inverse(world_to_local);

    for(const math::Vector& vertex : vertices)
        polygon_entity->AddVertex(math::Transform(world_to_local, vertex));

    return std::make_unique<PolygonProxy>(name, std::move(polygon_entity));
}
