
#include "WorldFile.h"
#include "Math/Serialize.h"

#include "Entity/IEntityManager.h"
#include "Component.h"
#include "Serialize.h"

#include "System/File.h"

#include "nlohmann/json.hpp"
#include <cstdio>
#include <cstring>


std::vector<uint32_t> world::ReadWorldComponentObjects(const char* file_name, IEntityManager* entity_manager)
{
    std::vector<uint32_t> created_entities;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return created_entities;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& entities = json["entities"];

    for(const auto& json_entity : entities)
    {
        const std::string& entity_name = json_entity["name"];
        const uint32_t entity_properties = json_entity["entity_properties"];

        mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), std::vector<uint32_t>());
        entity_manager->SetEntityProperties(new_entity.id, entity_properties);

        for(const auto& json_component : json_entity["components"])
        {
            Component component;
            component.name = json_component["name"];
            component.hash = mono::Hash(component.name.c_str());

            for(const nlohmann::json& property : json_component["properties"])
                component.properties.push_back(property);

            const Component& default_component = DefaultComponentFromHash(component.hash);
            UnionAttributes(component.properties, default_component.properties);

            entity_manager->AddComponent(new_entity.id, component.hash);
            entity_manager->SetComponentData(new_entity.id, component.hash, component.properties);
        }

        created_entities.push_back(new_entity.id);
    }

    return created_entities;
}
