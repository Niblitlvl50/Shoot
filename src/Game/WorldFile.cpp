
#include "WorldFile.h"
#include "Math/Serialize.h"
#include "Rendering/Serialize.h"

#include "EntitySystem/IEntityManager.h"
#include "EntitySystem/Serialize.h"
#include "Entity/Component.h"

#include "System/File.h"
#include "System/System.h"

#include "nlohmann/json.hpp"
#include <cstdio>
#include <cstring>

namespace
{
    game::LevelData ReadWorldComponents(
        const char* filename,
        mono::IEntityManager* entity_manager,
        game::EntityCreationCallback creation_callback)
    {
        System::Log("WorldFile|Loading world '%s'.", filename);
        const std::vector<byte> file_data = file::FileReadAll(filename);
        const nlohmann::json& json = nlohmann::json::parse(file_data);

        game::LevelData level_data;

        const bool has_metadata = json.contains("metadata");
        if(has_metadata)
        {
            const nlohmann::json& json_metadata = json["metadata"];
            level_data.metadata.level_name = json_metadata.value("level_name", "");
            level_data.metadata.level_description = json_metadata.value("level_description", "");
            level_data.metadata.level_game_mode = json_metadata.value("level_game_mode", "");
            level_data.metadata.camera_position = json_metadata.value("camera_position", math::ZeroVec);
            level_data.metadata.camera_size = json_metadata.value("camera_size", math::ZeroVec);
            level_data.metadata.player_spawn_point = json_metadata.value("player_spawn_point", math::ZeroVec);
            level_data.metadata.spawn_package = json_metadata.value("spawn_package", true);
            level_data.metadata.use_package_spawn_position = json_metadata.value("use_custom_package_spawn_position", false);
            level_data.metadata.package_spawn_position = json_metadata.value("package_spawn_position", math::ZeroVec);

            level_data.metadata.background_size = json_metadata.value("background_size", math::ZeroVec);
            level_data.metadata.background_color = json_metadata.value("background_color", mono::Color::BLACK);
            level_data.metadata.ambient_shade = json_metadata.value("ambient_shade", mono::Color::WHITE);
            level_data.metadata.background_texture = json_metadata.value("background_texture", "");
            level_data.metadata.background_music = json_metadata.value("background_music", std::string());
            level_data.metadata.triggers = json_metadata.value("triggers", std::vector<std::string>());

            level_data.metadata.navmesh_start = json_metadata.value("navmesh_start", math::ZeroVec);
            level_data.metadata.navmesh_end = json_metadata.value("navmesh_end", math::ZeroVec);
            level_data.metadata.navmesh_density = json_metadata.value("navmesh_density", 1.0f);

            level_data.metadata.time_limit_s = json_metadata.value("time_limit_s", 0);

            level_data.metadata.completed_trigger = json_metadata.value("completed_trigger", "");
            level_data.metadata.failed_trigger = json_metadata.value("failed_trigger", "");
        }

        const std::vector<mono::EntityData>& loaded_entity_data = json["entities"];
        
        std::vector<mono::Entity> loaded_entities;
        loaded_entities.reserve(loaded_entity_data.size());

        for(const mono::EntityData& entity_data : loaded_entity_data)
        {
            mono::Entity entity = entity_manager->CreateEntity(entity_data.entity_name.c_str(), entity_data.entity_uuid, { });
            entity_manager->SetEntityProperties(entity.id, entity_data.entity_properties);

            loaded_entities.push_back(entity);
        }

        for(uint32_t index = 0; index < loaded_entities.size(); ++index)
        {
            mono::Entity& entity = loaded_entities[index];
            const mono::EntityData& entity_data = loaded_entity_data[index];

            std::vector<Component> components;
            components.reserve(entity_data.entity_components.size());

            for(const mono::ComponentData& component_data : entity_data.entity_components)
            {
                Component component = component::DefaultComponentFromHash(component_data.hash);
                MergeAttributes(component.properties, component_data.properties);

                const bool add_component_result = entity_manager->AddComponent(entity.id, component.hash);
                const bool set_component_result = entity_manager->SetComponentData(entity.id, component.hash, component.properties);
                if(!add_component_result || !set_component_result)
                {
                    //System::Log("WorldFile|Failed to setup component with name '%s' for entity named '%s'", ComponentNameFromHash(component.hash), entity_name.c_str());
                }

                components.push_back(std::move(component));
            }

            level_data.loaded_entities.push_back(entity.id);

            if(creation_callback)
                creation_callback(entity, components);
        }

        return level_data;
    }
}

game::LevelData game::ReadWorldComponentObjects(
        const char* filename, mono::IEntityManager* entity_manager, game::EntityCreationCallback creation_callback)
{
    return ReadWorldComponents(filename, entity_manager, creation_callback);
}
