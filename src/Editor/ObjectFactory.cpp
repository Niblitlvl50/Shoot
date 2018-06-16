
#include "ObjectFactory.h"
#include "DefinedAttributes.h"

#include "ObjectProxies/EntityProxy.h"
#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PrefabProxy.h"

#include "Objects/Path.h"
#include "Objects/Polygon.h"
#include "Objects/Prefab.h"
#include "Objects/SpriteEntity.h"

#include "Math/Matrix.h"

using namespace editor;

ObjectFactory::ObjectFactory(Editor* editor)
    : m_editor(editor)
{
    m_repository.LoadDefinitions();
}

IObjectProxyPtr ObjectFactory::CreateObject(const char* object_name) const
{
    IObjectProxyPtr new_object = CreateEntity(object_name);
    if(new_object)
        return new_object;

    return CreatePrefab(object_name);
}

IObjectProxyPtr ObjectFactory::CreateEntity(const char* entity_name) const
{
    const EntityDefinition* def = m_repository.GetDefinitionFromName(entity_name);
    if(!def)
        return nullptr;
    
    auto entity = std::make_shared<editor::SpriteEntity>(entity_name, def->sprite_file.c_str());
    entity->SetScale(def->scale);

    std::vector<Attribute> attributes;
    for(unsigned int hash : def->attribute_types)
        attributes.push_back({hash, world::DefaultAttributeFromHash(hash)});
    
    return std::make_unique<EntityProxy>(entity, attributes);
}

IObjectProxyPtr ObjectFactory::CreatePath(const std::string& name, const std::vector<math::Vector>& points) const
{
    auto path = std::make_shared<editor::PathEntity>(name, points);
    return std::make_unique<PathProxy>(path, m_editor);
}

IObjectProxyPtr ObjectFactory::CreatePrefab(const std::string& prefab_name) const
{
    const PrefabDefinition* def = m_repository.GetPrefabFromName(prefab_name);
    if(!def)
        return nullptr;

    auto prefab_object = std::make_shared<editor::Prefab>(prefab_name.c_str(), def->sprite_file.c_str(), def->snap_points);
    prefab_object->SetScale(def->scale);

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
