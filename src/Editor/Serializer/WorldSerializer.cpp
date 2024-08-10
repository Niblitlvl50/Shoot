
#include "WorldSerializer.h"
#include "ObjectProxies/ComponentProxy.h"

#include "EntitySystem/Entity.h"
#include "EntitySystem/Serialize.h"
#include "WorldFile.h"
#include "Entity/Component.h"

#include "System/File.h"

#include "nlohmann/json.hpp"


editor::World editor::LoadWorld(
    const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor)
{
    editor::World world;

    const auto entity_callback =
        [&world, entity_manager, transform_system, editor]
        (const mono::Entity& entity, const std::vector<Component>& components)
    {
        auto component_proxy = std::make_unique<ComponentProxy>(entity.id, components, entity_manager, transform_system, editor);
        world.loaded_proxies.push_back(std::move(component_proxy));
    };

    world.leveldata = game::ReadWorldComponentObjects(file_name, entity_manager, entity_callback);
    return world;
}

void editor::SaveWorld(const char* file_name, const std::vector<const IObjectProxy*>& proxies, const game::LevelMetadata& level_data)
{
    WriteComponentEntities(file_name, level_data, proxies);
}

void editor::WriteComponentEntities(
    const std::string& file_path, const game::LevelMetadata& level_metadata, const std::vector<const IObjectProxy*>& proxies)
{
    std::vector<nlohmann::json> json_entities;

    for(const IObjectProxy* proxy : proxies)
    {
        nlohmann::json json_components;

        for(const Component& component : proxy->GetComponents())
        {
            nlohmann::json component_properties;
            
            for(const Attribute& property : component.properties)
                component_properties.push_back(property);
            
            nlohmann::json json_component;
            json_component["hash"] = component.hash;
            json_component["name"] = component::ComponentNameFromHash(component.hash);
            json_component["properties"] = component_properties;

            json_components.push_back(json_component);
        }

        nlohmann::json json_entity;
        json_entity["uuid_hash"] = proxy->Uuid();
        json_entity["name"] = proxy->Name();
        json_entity["entity_properties"] = proxy->GetEntityProperties();
        json_entity["components"] = json_components;

        json_entities.push_back(json_entity);
    }

    nlohmann::json json_metadata;
    json_metadata["level_name"] = level_metadata.level_name;
    json_metadata["level_description"] = level_metadata.level_description;
    json_metadata["level_game_mode"] = level_metadata.level_game_mode;
    json_metadata["camera_position"] = level_metadata.camera_position;
    json_metadata["camera_size"] = level_metadata.camera_size;
    json_metadata["player_spawn_point"] = level_metadata.player_spawn_point;
    json_metadata["spawn_package"] = level_metadata.spawn_package;
    json_metadata["use_custom_package_spawn_position"] = level_metadata.use_package_spawn_position;
    json_metadata["package_spawn_position"] = level_metadata.package_spawn_position;

    json_metadata["background_color"] = level_metadata.background_color;
    json_metadata["ambient_shade"] = level_metadata.ambient_shade;
    json_metadata["background_size"] = level_metadata.background_size;
    json_metadata["background_texture"] = level_metadata.background_texture;
    json_metadata["background_music"] = level_metadata.background_music;
    json_metadata["triggers"] = level_metadata.triggers;
    json_metadata["navmesh_start"] = level_metadata.navmesh_start;
    json_metadata["navmesh_end"] = level_metadata.navmesh_end;
    json_metadata["navmesh_density"] = level_metadata.navmesh_density;

    json_metadata["completed_trigger"] = level_metadata.completed_trigger;
    json_metadata["completed_alt_trigger"] = level_metadata.completed_alt_trigger;
    json_metadata["aborted_trigger"] = level_metadata.aborted_trigger;
    json_metadata["failed_trigger"] = level_metadata.failed_trigger;

    nlohmann::json json;
    json["metadata"] = json_metadata;
    json["entities"] = json_entities;

    const std::string& serialized_json = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}
