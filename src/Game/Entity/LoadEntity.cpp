
#include "LoadEntity.h"

#include "Component.h"
#include "Serialize.h"

#include "System/File.h"
#include "nlohmann/json.hpp"

std::vector<mono::EntityData> game::LoadEntityFile(const char* entity_file)
{
    std::vector<mono::EntityData> loaded_entities;

    file::FilePtr file = file::OpenAsciiFile(entity_file);
    if(!file)
        return loaded_entities;

    std::vector<byte> file_data = file::FileRead(file);
    file_data.push_back('\0');

    const nlohmann::json& json = nlohmann::json::parse((const char*)file_data.data());
    const nlohmann::json& entities = json["entities"];

    for(const nlohmann::json& entity : entities)
    {
        mono::EntityData entity_data;
        entity_data.entity_uuid = entity.value("uuid_hash", 0);
        entity_data.entity_name = entity.value("name", "Unnamed");
        entity_data.entity_properties = entity.value("entity_properties", 0);

        for(const nlohmann::json& component : entity["components"])
        {
            mono::ComponentData component_data;
            component_data.name = component["name"];

            for(const nlohmann::json& property : component["properties"])
                component_data.properties.push_back(property);

            entity_data.entity_components.push_back(std::move(component_data));
        }

        loaded_entities.push_back(entity_data);
    }

    return loaded_entities;
}
