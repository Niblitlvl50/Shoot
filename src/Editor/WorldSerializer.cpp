
#include "WorldSerializer.h"
#include "Objects/Polygon.h"
#include "Objects/Path.h"
#include "Math/Matrix.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "System/File.h"
#include "WorldFile.h"

#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "Entity/IEntityManager.h"
#include "ObjectProxies/ComponentProxy.h"
#include "Component.h"
#include "Serialize.h"

#include "ObjectProxies/IObjectProxy.h"

#include "nlohmann/json.hpp"

#include "ObjectFactory.h"

#include "Serializer/JsonSerializer.h"

#include <algorithm>

std::vector<IObjectProxyPtr> editor::LoadPolygons(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> proxies;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return proxies;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& polygons = json["polygons"];

    for(const auto& json_polygon : polygons)
    {
        const std::string name = json_polygon["name"];
        const math::Vector position = json_polygon["position"];
        const math::Vector base_point = json_polygon["base_point"];
        const float rotation = json_polygon["rotation"];
        const std::string texture = json_polygon["texture"];
        const std::vector<math::Vector> vertices = json_polygon["vertices"];

        auto proxy = factory.CreatePolygon(name, position, base_point, rotation, vertices, texture);
        proxies.push_back(std::move(proxy));
    }

    return proxies;
}

std::vector<IObjectProxyPtr> editor::LoadPaths(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> paths;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return paths;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const std::vector<std::string>& path_names = json["path_files"];

    paths.reserve(path_names.size());

    const auto get_name = [](const std::string& file_name) {
        const size_t slash_pos = file_name.find_last_of('/');
        const size_t dot_pos = file_name.find_last_of('.');
        return file_name.substr(slash_pos +1, dot_pos - slash_pos -1);
    };

    for(const std::string& file : path_names)
    {
        mono::IPathPtr path = mono::CreatePath(file.c_str());
        auto proxy = factory.CreatePath(get_name(file), path->GetPathPoints());
        proxy->Entity()->SetPosition(path->GetGlobalPosition());

        paths.push_back(std::move(proxy));
    }

    return paths;
}

std::vector<IObjectProxyPtr> editor::LoadComponentObjects(const char* file_name, IEntityManager* entity_manager, mono::TransformSystem* transform_system)
{
    std::vector<IObjectProxyPtr> proxies;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return proxies;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& entities = json["entities"];

    for(const auto& json_entity : entities)
    {
        const std::string& entity_name = json_entity["name"];
        mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), std::vector<uint32_t>());

        std::vector<Component> components;

        for(const auto& json_component : json_entity["components"])
        {
            Component component;
            component.name = json_component["name"];
            component.hash = mono::Hash(component.name.c_str());

            for(const nlohmann::json& property : json_component["properties"])
                component.properties.push_back(property);

            const Component& default_component = DefaultComponentFromHash(component.hash);
            UnionAttributes(component.properties, default_component.properties);

            components.push_back(std::move(component));
        }

        const std::string entity_folder = json_entity.value("folder", ""); 
        const uint32_t entity_props = json_entity.value("entity_properties", 0);

        auto component_proxy = std::make_unique<ComponentProxy>(new_entity.id, entity_name, entity_folder, components, entity_manager, transform_system);
        component_proxy->SetEntityProperties(entity_props);

        proxies.push_back(std::move(component_proxy));
    }

    return proxies;
}

std::vector<IObjectProxyPtr> editor::LoadWorld(
    const char* file_name, const editor::ObjectFactory& factory, IEntityManager* entity_manager, mono::TransformSystem* transform_system)
{
    std::vector<IObjectProxyPtr> world_objects;

    auto component_objects = LoadComponentObjects("res/world.components", entity_manager, transform_system);
    for(auto& proxy : component_objects)
        world_objects.push_back(std::move(proxy));

    auto paths = LoadPaths("res/world.paths", factory);
    for(auto& proxy : paths)
        world_objects.push_back(std::move(proxy));

    auto polygons = LoadPolygons("res/world.polygons", factory);
    for(auto& proxy : polygons)
        world_objects.push_back(std::move(proxy));

    return world_objects;
}

void editor::SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies)
{
    JsonSerializer serializer;
    
    for(auto& proxy : proxies)
        proxy->Visit(serializer);

    serializer.WriteEntities("res/world.objects");
    serializer.WriteComponentEntities("res/world.components");
    serializer.WritePathFile("res/world.paths");
    serializer.WritePolygons("res/world.polygons");
}
