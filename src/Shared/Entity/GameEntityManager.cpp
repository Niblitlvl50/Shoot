
#include "GameEntityManager.h"
#include "Component.h"

#include "Serialize.h"

#include "System/File.h"
#include "nlohmann/json.hpp"

using namespace shared;

mono::EntityData GameEntityManager::LoadEntityFile(const char* entity_file) const
{
    file::FilePtr file = file::OpenAsciiFile(entity_file);
    if(!file)
        return { };

    std::vector<byte> file_data = file::FileRead(file);
    file_data.push_back('\0');

    const nlohmann::json& json = nlohmann::json::parse((const char*)file_data.data());
    //const nlohmann::json& entities = json["entities"];
    const nlohmann::json& first_entity = json["entities"][0];

    mono::EntityData entity_data;
    entity_data.entity_name = first_entity.value("name", "Unnamed");
    entity_data.entity_properties = first_entity.value("entity_properties", 0);

    for(const nlohmann::json& component : first_entity["components"])
    {
        mono::ComponentData component_data;
        component_data.name = component["name"];

        for(const nlohmann::json& property : component["properties"])
            component_data.properties.push_back(property);

        entity_data.entity_components.push_back(std::move(component_data));
    }

    return entity_data;
}

const char* GameEntityManager::ComponentNameFromHash(uint32_t component_hash) const
{
    return ::ComponentNameFromHash(component_hash);
}
