
#include "JsonSerializer.h"

#include "ObjectProxies/ComponentProxy.h"

#include "System/File.h"

#include "Entity/Component.h"
#include "Entity/Serialize.h"

#include "nlohmann/json.hpp"

using namespace editor;

void JsonSerializer::WriteComponentEntities(const std::string& file_path, const game::LevelMetadata& level_metadata) const
{
    nlohmann::json json_metadata;
    json_metadata["level_name"] = level_metadata.level_name;
    json_metadata["level_description"] = level_metadata.level_description;
    json_metadata["camera_position"] = level_metadata.camera_position;
    json_metadata["camera_size"] = level_metadata.camera_size;
    json_metadata["player_spawn_point"] = level_metadata.player_spawn_point;
    json_metadata["background_color"] = level_metadata.background_color;
    json_metadata["ambient_shade"] = level_metadata.ambient_shade;
    json_metadata["background_size"] = level_metadata.background_size;
    json_metadata["background_texture"] = level_metadata.background_texture;
    json_metadata["background_music"] = level_metadata.background_music;
    json_metadata["triggers"] = level_metadata.triggers;
    json_metadata["navmesh_start"] = level_metadata.navmesh_start;
    json_metadata["navmesh_end"] = level_metadata.navmesh_end;
    json_metadata["navmesh_density"] = level_metadata.navmesh_density;
    json_metadata["time_limit_s"] = level_metadata.time_limit_s;

    nlohmann::json json;
    json["metadata"] = json_metadata;
    json["entities"] = m_json_entities_components;

    const std::string& serialized_json = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::Accept(ComponentProxy* proxy)
{
    nlohmann::json json_components;

    for(const Component& component : proxy->GetComponents())
    {
        nlohmann::json component_properties;
        
        for(const Attribute& property : component.properties)
            component_properties.push_back(property);
        
        nlohmann::json json_component;
        json_component["hash"] = component.hash;
        json_component["name"] = ComponentNameFromHash(component.hash);
        json_component["properties"] = component_properties;

        json_components.push_back(json_component);
    }

    nlohmann::json json_entity;
    json_entity["uuid_hash"] = proxy->Uuid();
    json_entity["name"] = proxy->Name();
    json_entity["folder"] = proxy->GetFolder();
    json_entity["entity_properties"] = proxy->GetEntityProperties();
    json_entity["components"] = json_components;

    m_json_entities_components.push_back(json_entity);
}
