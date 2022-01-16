
#include "WorldFile.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"

#include "EntitySystem/IEntityManager.h"
#include "Entity/Component.h"
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

        System::Log("WorldFile|Loading world '%s'.", filename);
        const std::vector<byte> file_data = file::FileReadAll(filename);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        const bool has_metadata = json.contains("metadata");
        if(has_metadata)
        {
            const nlohmann::json& json_metadata = json["metadata"];
            level_data.metadata.camera_position = json_metadata.value("camera_position", math::ZeroVec);
            level_data.metadata.camera_size = json_metadata.value("camera_size", math::ZeroVec);
            level_data.metadata.player_spawn_point = json_metadata.value("player_spawn_point", math::ZeroVec);
            level_data.metadata.background_size = json_metadata.value("background_size", math::ZeroVec);
            level_data.metadata.background_color = json_metadata.value("background_color", mono::Color::BLACK);
            level_data.metadata.ambient_shade = json_metadata.value("ambient_shade", mono::Color::WHITE);
            level_data.metadata.background_texture = json_metadata.value("background_texture", "");
            level_data.metadata.triggers = json_metadata.value("triggers", std::vector<std::string>());

            level_data.metadata.navmesh_start = json_metadata.value("navmesh_start", math::ZeroVec);
            level_data.metadata.navmesh_end = json_metadata.value("navmesh_end", math::ZeroVec);
            level_data.metadata.navmesh_density = json_metadata.value("navmesh_density", 1.0f);
        }

        const nlohmann::json& json_entities = json["entities"];
        for(uint32_t index = 0; index < json_entities.size(); ++index)
        {
            const auto& json_entity = json_entities[index];

            const bool has_uuid_hash = json_entity.contains("uuid_hash");
            const std::string& entity_name = json_entity["name"];
            const std::string& entity_folder = json_entity.value("folder", ""); 
            const uint32_t entity_properties = json_entity["entity_properties"];

            mono::Entity new_entity;

            if(has_uuid_hash)
            {
                const uint32_t uuid_hash = json_entity["uuid_hash"];
                new_entity = entity_manager->CreateEntity(entity_name.c_str(), uuid_hash, { });
            }
            else
            {
                new_entity = entity_manager->CreateEntity(entity_name.c_str(), { });
            }

            entity_manager->SetEntityProperties(new_entity.id, entity_properties);

            std::vector<Component> components;
            std::vector<uint32_t> ignored_components;

            const nlohmann::json& json_components = json_entity["components"];
            for(const auto& json_component : json_components)
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

            // Patch the exported entities to have name and folder component, remove later.
            Component* name_folder = FindComponentFromHash(NAME_FOLDER_COMPONENT, components);
            if(!name_folder)
            {
                Component name_folder_component = DefaultComponentFromHash(NAME_FOLDER_COMPONENT);
                SetAttribute(NAME_ATTRIBUTE, name_folder_component.properties, entity_name);
                components.push_back(name_folder_component);
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
