
#include "WorldSerializer.h"

#include "Math/Matrix.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"

#include "System/File.h"
#include "System/System.h"

#include "ObjectProxies/PathProxy.h"
#include "WorldFile.h"

#include "EntitySystem/IEntityManager.h"
#include "ObjectProxies/IObjectProxy.h"
#include "ObjectProxies/ComponentProxy.h"
#include "Component.h"
#include "Entity/Serialize.h"

#include "Serializer/JsonSerializer.h"

#include "nlohmann/json.hpp"
#include <algorithm>


std::vector<IObjectProxyPtr> editor::LoadComponentObjects(
    const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
{
    std::vector<IObjectProxyPtr> proxies;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return proxies;

    const std::vector<byte> file_data = file::FileRead(file);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& entities = json["entities"];

    for(const auto& json_entity : entities)
    {
        const std::string& entity_name = json_entity["name"];
        //const std::string& entity_folder = json_entity.value("folder", "");
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
            {
                //System::Log("WorldSerializer|Failed to setup component with name '%s' for entity named '%s'", ComponentNameFromHash(component.hash), entity_name.c_str());
            }
        }

        // Patch the exported entities to have name and folder component, remove later.
        Component* name_folder = FindComponentFromHash(NAME_FOLDER_COMPONENT, components);
        if(!name_folder)
        {
            Component name_folder_component = DefaultComponentFromHash(NAME_FOLDER_COMPONENT);
            SetAttribute(NAME_ATTRIBUTE, name_folder_component.properties, entity_name);
            components.push_back(name_folder_component);
        }

        auto component_proxy = std::make_unique<ComponentProxy>(new_entity.id, components, entity_manager, transform_system, editor);
        component_proxy->SetEntityProperties(entity_properties);

        proxies.push_back(std::move(component_proxy));
    }

    return proxies;
}

editor::World editor::LoadWorld(
    const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
{
    editor::World world;

    const auto entity_callback =
        [&world, entity_manager, transform_system, editor]
        (const mono::Entity& entity, const std::string& folder, const std::vector<Component>& components)
    {
        auto component_proxy = std::make_unique<ComponentProxy>(entity.id, components, entity_manager, transform_system, editor);
        component_proxy->SetEntityProperties(entity.properties);

        world.loaded_proxies.push_back(std::move(component_proxy));
    };

    world.leveldata = shared::ReadWorldComponentObjects(file_name, entity_manager, entity_callback);
    return world;
}

void editor::SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies, const shared::LevelMetadata& level_data)
{
    JsonSerializer serializer;

    for(auto& proxy : proxies)
        proxy->Visit(serializer);

    serializer.WriteComponentEntities(file_name, level_data);
}
