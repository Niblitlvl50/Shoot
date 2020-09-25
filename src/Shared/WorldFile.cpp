
#include "WorldFile.h"
#include "Math/Serialize.h"

#include "EntitySystem/IEntityManager.h"
#include "Component.h"
#include "Serialize.h"

#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"
#include <cstdio>
#include <cstring>


shared::LevelData shared::ReadWorldComponentObjects(
    const char* file_name, mono::IEntityManager* entity_manager, EntityCreationCallback callback)
{
    LevelData level_data;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return level_data;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);

    const nlohmann::json& json_metadata = json["metadata"];
    level_data.metadata.camera_position = json_metadata["camera_position"];
    level_data.metadata.camera_size = json_metadata["camera_size"];

    const nlohmann::json& entities = json["entities"];

    for(const auto& json_entity : entities)
    {
        const std::string& entity_name = json_entity["name"];
        const std::string& entity_folder = json_entity.value("folder", ""); 
        const uint32_t entity_properties = json_entity["entity_properties"];

        mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), std::vector<uint32_t>());
        entity_manager->SetEntityProperties(new_entity.id, entity_properties);

        std::vector<Component> components;

        for(const auto& json_component : json_entity["components"])
        {
            const uint32_t component_hash = json_component["hash"];
            const std::string component_name = json_component["name"];

            std::vector<Attribute> loaded_properties;
            for(const nlohmann::json& property : json_component["properties"])
                loaded_properties.push_back(property);

            Component component = DefaultComponentFromHash(component_hash);
            MergeAttributes(component.properties, loaded_properties);
            components.push_back(std::move(component));
        }

        for(const Component& component : components)
        {
            const bool add_component_result = entity_manager->AddComponent(new_entity.id, component.hash);
            const bool set_component_result = entity_manager->SetComponentData(new_entity.id, component.hash, component.properties);
            if(!add_component_result || !set_component_result)
                System::Log("Failed to setup component with name '%s' for entity named '%s'\n", ComponentNameFromHash(component.hash), entity_name.c_str());
        }

        level_data.loaded_entities.push_back(new_entity.id);

        if(callback)
            callback(new_entity, entity_folder, components);
    }

    return level_data;
}
