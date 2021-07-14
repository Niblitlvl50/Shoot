
#include "WorldFile.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"

#include "EntitySystem/IEntityManager.h"
#include "Component.h"
#include "Entity/Serialize.h"

#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"
#include <cstdio>
#include <cstring>

namespace
{
    shared::LevelData ReadWorldComponents(
        const char* filename,
        mono::IEntityManager* entity_manager,
        shared::ComponentFilterCallback component_filter,
        shared::EntityCreationCallback creation_callback)
    {
        shared::LevelData level_data;

        file::FilePtr file = file::OpenAsciiFile(filename);
        if(!file)
            return level_data;

        System::Log("WorldFile|Loading world '%s'.", filename);

        const std::vector<byte> file_data = file::FileRead(file);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        const nlohmann::json& json_metadata = json["metadata"];
        level_data.metadata.camera_position = json_metadata["camera_position"];
        level_data.metadata.camera_size = json_metadata["camera_size"];
        level_data.metadata.player_spawn_point = json_metadata["player_spawn_point"];
        level_data.metadata.background_color = json_metadata.value("background_color", mono::Color::BLACK);
        level_data.metadata.ambient_shade = json_metadata.value("ambient_shade", mono::Color::WHITE);
        level_data.metadata.background_texture = json_metadata["background_texture"];

        const nlohmann::json& entities = json["entities"];

        for(const auto& json_entity : entities)
        {
            const uint32_t uuid_hash = json_entity["uuid_hash"];
            const std::string& entity_name = json_entity["name"];
            const std::string& entity_folder = json_entity.value("folder", ""); 
            const uint32_t entity_properties = json_entity["entity_properties"];

            mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), uuid_hash, std::vector<uint32_t>());
            entity_manager->SetEntityProperties(new_entity.id, entity_properties);

            std::vector<Component> components;
            std::vector<uint32_t> ignored_components;

            for(const auto& json_component : json_entity["components"])
            {
                const uint32_t component_hash = json_component["hash"];
                const std::string component_name = json_component["name"];

                if(component_filter)
                {
                    const bool accept_component = component_filter(component_hash);
                    if(!accept_component)
                    {
                        ignored_components.push_back(component_hash);
                        continue;
                    }
                }

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
                {
                    //System::Log("WorldFile|Failed to setup component with name '%s' for entity named '%s'", ComponentNameFromHash(component.hash), entity_name.c_str());
                }
            }

            if(!ignored_components.empty())
            {
                std::string component_text;
                for(uint32_t component_hash : ignored_components)
                    component_text += ComponentNameFromHash(component_hash) + std::string("|");
                component_text.pop_back();

                System::Log(
                    "Ignored the following components for entity named '%s', %s", entity_name.c_str(), component_text.c_str());
            }

            level_data.loaded_entities.push_back(new_entity.id);

            if(creation_callback)
                creation_callback(new_entity, entity_folder, components);
        }

        return level_data;
    }
}

shared::LevelData shared::ReadWorldComponentObjects(
        const char* filename, mono::IEntityManager* entity_manager, EntityCreationCallback creation_callback)
{
    return ReadWorldComponents(filename, entity_manager, nullptr, creation_callback);
}

shared::LevelData shared::ReadWorldComponentObjectsFiltered(
        const char* filename, mono::IEntityManager* entity_manager, ComponentFilterCallback component_filter)
{
    return ReadWorldComponents(filename, entity_manager, component_filter, nullptr);
}
