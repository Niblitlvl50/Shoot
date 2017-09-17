
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

#include "nlohmann_json/json.hpp"

using namespace editor;

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
