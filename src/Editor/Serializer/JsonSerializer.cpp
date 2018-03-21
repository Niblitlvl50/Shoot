
#include "JsonSerializer.h"

#include "ObjectProxies/EntityProxy.h"
#include "ObjectProxies/PathProxy.h"
#include "ObjectProxies/PolygonProxy.h"
#include "ObjectProxies/PrefabProxy.h"

#include "Objects/Path.h"
#include "Objects/Prefab.h"

#include "Math/Serialize.h"
#include "System/File.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "DefinedAttributes.h"

#include "nlohmann_json/json.hpp"

using namespace editor;

void JsonSerializer::WriteEntities(const std::string& file_path) const
{
    const std::string& serialized_json = m_json_entities.dump(4);

    File::FilePtr file = File::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::WritePathFile(const std::string& file_path) const
{
    nlohmann::json json;
    json["path_files"] = m_path_names;

    const std::string& serialized_json = json.dump(4);

    File::FilePtr file = File::CreateAsciiFile(file_path.c_str());
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
    
    File::FilePtr file = File::CreateAsciiFile(file_path.c_str());
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void JsonSerializer::Accept(EntityProxy* proxy)
{
    nlohmann::json json_attributes;

    for(const auto& attribute : proxy->GetAttributes())
    {
        nlohmann::json json_object;
        json_object["name"] = world::AttributeNameFromHash(attribute.id);

        switch(attribute.attribute.type)
        {
        case Variant::Type::INT:
            json_object["value"] = (int)attribute.attribute;
            break;    
        case Variant::Type::FLOAT:
            json_object["value"] = (float)attribute.attribute;
            break;    
        case Variant::Type::STRING:
            json_object["value"] = (const char*)attribute.attribute;
            break;    
        case Variant::Type::POINT:
            json_object["value"] = (math::Vector)attribute.attribute;
            break;
        default:
            break;
        }

        json_attributes.push_back(json_object);
    }

    nlohmann::json json_entity;
    json_entity["name"] = proxy->Name();
    json_entity["attributes"] = json_attributes;

    m_json_entities.push_back(json_entity);
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
