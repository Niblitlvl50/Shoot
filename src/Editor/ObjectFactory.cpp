
#include "ObjectFactory.h"

#include "ObjectProxies/EntityProxy.h"
#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PrefabProxy.h"

#include "Objects/Path.h"
#include "Objects/Polygon.h"
#include "Objects/Prefab.h"
#include "Objects/SpriteEntity.h"

#include "Math/Matrix.h"

#include "Hash.h"

using namespace editor;

ObjectFactory::ObjectFactory(Editor* editor)
    : m_editor(editor)
{
    m_repository.LoadDefinitions();
}

IObjectProxyPtr ObjectFactory::CreateObject(const char* object_name) const
{
    const EntityDefinition def = m_repository.GetDefinitionFromName(object_name);
    auto entity = std::make_shared<editor::SpriteEntity>(object_name, def.sprite_file.c_str());
    entity->SetScale(def.scale);
    

    auto child = std::make_shared<SpriteEntity>(object_name, def.sprite_file.c_str());
    child->SetPosition(math::Vector(2, 1));
    entity->AddChild(child);


    return std::make_unique<EntityProxy>(entity, def.attribute_types);
}

IObjectProxyPtr ObjectFactory::CreatePath(const std::string& name, const std::vector<math::Vector>& points) const
{
    auto path = std::make_shared<editor::PathEntity>(name, points);
    return std::make_unique<PathProxy>(path, m_editor);
}

IObjectProxyPtr ObjectFactory::CreatePrefab(const std::string& prefab_name) const
{
    const PrefabDefinition& def = m_repository.GetPrefabFromName(prefab_name);
    auto prefab_object = std::make_shared<editor::Prefab>(prefab_name.c_str(), def.sprite_file.c_str(), def.snap_points);
    prefab_object->SetScale(def.scale);

    return std::make_unique<PrefabProxy>(prefab_object);
}

IObjectProxyPtr ObjectFactory::CreatePolygon(
    const math::Vector& position,
    const math::Vector& base_point,
    float rotation,
    const std::vector<math::Vector>& vertices,
    const std::string& texture_name) const
{
    auto polygon_entity = std::make_shared<editor::PolygonEntity>();
    polygon_entity->SetPosition(position);
    polygon_entity->SetBasePoint(base_point);
    polygon_entity->SetRotation(rotation);
    polygon_entity->SetTexture(texture_name.c_str());

    math::Matrix world_to_local = polygon_entity->Transformation();
    math::Inverse(world_to_local);

    for(const math::Vector& vertex : vertices)
        polygon_entity->AddVertex(math::Transform(world_to_local, vertex));

    return std::make_unique<PolygonProxy>(polygon_entity);
}
