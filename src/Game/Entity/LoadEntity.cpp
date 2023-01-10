
#include "LoadEntity.h"
#include "EntitySystem/EntityTypes.h"
#include "EntitySystem/Serialize.h"

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
        const mono::EntityData& entity_data = entity;
        loaded_entities.push_back(entity_data);
    }

    return loaded_entities;
}
