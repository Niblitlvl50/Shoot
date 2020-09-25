
#include "WorldSerializer.h"

#include "Math/Matrix.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "System/File.h"
#include "System/System.h"

#include "Objects/Polygon.h"
#include "Objects/Path.h"

#include "WorldFile.h"

#include "EntitySystem/IEntityManager.h"
#include "ObjectProxies/IObjectProxy.h"
#include "ObjectProxies/ComponentProxy.h"
#include "Component.h"
#include "Serialize.h"

#include "ObjectFactory.h"
#include "Serializer/JsonSerializer.h"

#include "nlohmann/json.hpp"
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

std::vector<IObjectProxyPtr> editor::LoadComponentObjects(
    const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
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
        const std::string& entity_folder = json_entity.value("folder", "");
        const uint32_t entity_properties = json_entity.value("entity_properties", 0);

        mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), std::vector<uint32_t>());
        entity_manager->SetEntityProperties(new_entity.id, entity_properties);

        std::vector<Component> components;

        for(const auto& json_component : json_entity["components"])
        {
            const uint32_t component_hash = json_component["hash"];
            const std::string component_name = json_component["name"];
            
            std::vector<Attribute> loaded_attributes;
            for(const nlohmann::json& property : json_component["properties"])
                loaded_attributes.push_back(property);

            Component component = DefaultComponentFromHash(component_hash);
            MergeAttributes(component.properties, loaded_attributes);
            components.push_back(std::move(component));
        }

        for(const Component& component : components)
        {
            const bool add_component_result = entity_manager->AddComponent(new_entity.id, component.hash);
            const bool set_component_result = entity_manager->SetComponentData(new_entity.id, component.hash, component.properties);
            if(!add_component_result || !set_component_result)
                System::Log("Failed to setup component with name '%s' for entity named '%s'\n", ComponentNameFromHash(component.hash), entity_name.c_str());
        }

        auto component_proxy =
            std::make_unique<ComponentProxy>(new_entity.id, entity_name, entity_folder, components, entity_manager, transform_system, editor);
        component_proxy->SetEntityProperties(entity_properties);

        proxies.push_back(std::move(component_proxy));
    }

    return proxies;
}

editor::World editor::LoadWorld(
    const char* file_name, const editor::ObjectFactory& factory, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
{
    editor::World world;

    const auto entity_callback =
        [&world, entity_manager, transform_system, editor]
        (const mono::Entity& entity, const std::string& folder, const std::vector<Component>& components)
    {
        auto component_proxy =
            std::make_unique<ComponentProxy>(entity.id, entity.name, folder, components, entity_manager, transform_system, editor);
        component_proxy->SetEntityProperties(entity.properties);

        world.loaded_proxies.push_back(std::move(component_proxy));
    };

    world.leveldata = shared::ReadWorldComponentObjects(file_name, entity_manager, entity_callback);

    auto paths = LoadPaths("res/world.paths", factory);
    for(auto& proxy : paths)
        world.loaded_proxies.push_back(std::move(proxy));

    auto polygons = LoadPolygons("res/world.polygons", factory);
    for(auto& proxy : polygons)
        world.loaded_proxies.push_back(std::move(proxy));

    return world;
}

void editor::SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies, const shared::LevelMetadata& level_data)
{
    JsonSerializer serializer;

    for(auto& proxy : proxies)
        proxy->Visit(serializer);

    serializer.WriteComponentEntities(file_name, level_data);
    serializer.WritePathFile("res/world.paths");
    serializer.WritePolygons("res/world.polygons");
}
