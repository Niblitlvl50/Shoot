
#include "JsonSerializer.h"

#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PrefabProxy.h"
#include "ObjectProxies/ComponentProxy.h"

#include "Objects/Path.h"
#include "Objects/Prefab.h"

#include "Math/Serialize.h"
#include "Rendering/Serialize.h"

#include "System/File.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "DefinedAttributes.h"
#include "Component.h"
#include "Serialize.h"

#include "nlohmann/json.hpp"

using namespace editor;

void JsonSerializer::WriteEntities(const std::string& file_path) const
{
    nlohmann::json json;
    json["objects"] = m_json_entities;

    const std::string& serialized_json = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::WriteComponentEntities(const std::string& file_path) const
{
    nlohmann::json json;
    json["entities"] = m_json_entities_components;

    const std::string& serialized_json = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::WritePathFile(const std::string& file_path) const
{
    nlohmann::json json;
    json["path_files"] = m_path_names;

    const std::string& serialized_json = json.dump(4);

    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::WritePrefabs(const std::string& file_path) const
{
    nlohmann::json json_prefabs_list;

    for(const auto& prefab_data : m_prefabs)
    {
        nlohmann::json json_prefab;
        json_prefab["name"] = prefab_data.name;
        json_prefab["position"] = prefab_data.position;
        json_prefab["rotation"] = prefab_data.rotation;
    
        json_prefabs_list.push_back(json_prefab);
    }

    nlohmann::json json;
    json["prefabs"] = json_prefabs_list;

    const std::string& serialized_json = json.dump(4);
    
    file::FilePtr file = file::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::Accept(PathProxy* proxy)
{
    auto path = proxy->m_path;

    const std::string& filename = "res/paths/" + path->m_name + ".path";
    mono::SavePath(filename.c_str(), path->Position(), path->m_points);

    m_path_names.push_back(filename);
}

void JsonSerializer::Accept(PolygonProxy* proxy)
{

}

void JsonSerializer::Accept(PrefabProxy* proxy)
{
    PrefabData data;
    data.name = proxy->m_prefab->Name();
    data.position = proxy->m_prefab->Position();
    data.rotation = proxy->m_prefab->Rotation();

    m_prefabs.push_back(data);
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
        json_component["name"] = component.name;
        json_component["properties"] = component_properties;

        json_components.push_back(json_component);
    }

    nlohmann::json json_entity;
    json_entity["name"] = proxy->Name();
    json_entity["components"] = json_components;

    m_json_entities_components.push_back(json_entity);
}
