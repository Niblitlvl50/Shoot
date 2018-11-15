
#include "Prefabs.h"

#include "System/File.h"
#include "Math/Serialize.h"
#include "nlohmann/json.hpp"

std::vector<PrefabDefinition> LoadPrefabDefinitions()
{
    std::vector<PrefabDefinition> loaded_prefabs;

    File::FilePtr file = File::OpenAsciiFile("res/editor_prefabs.json");
    if(!file)
        return loaded_prefabs;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);
    const nlohmann::json& json = nlohmann::json::parse(file_data);

    const nlohmann::json& prefabs = json["prefabs"];
    loaded_prefabs.reserve(prefabs.size());

    for(const auto& prefab : prefabs)
    {
        PrefabDefinition definition;
        definition.name = prefab["name"];
        definition.sprite_file = prefab["sprite"];
        definition.scale = prefab["scale"];

        const nlohmann::json& collision_shape = prefab["collision_shape"];
        definition.collision_shape.reserve(collision_shape.size());

        for(const auto& point : collision_shape)
            definition.collision_shape.push_back(point);

        const nlohmann::json& snap_points = prefab["snap_points"];
        definition.snap_points.reserve(snap_points.size());

        for(const auto& snap_point : snap_points)
        {
            SnapPoint sp;
            sp.normal = snap_point["normal"];
            sp.position = snap_point["position"];
            definition.snap_points.push_back(sp);
        }

        loaded_prefabs.push_back(definition);
    }

    return loaded_prefabs;
}

const PrefabDefinition* FindPrefabFromName(const std::string& prefab_name, const std::vector<PrefabDefinition>& prefabs)
{
    const auto find_by_name = [&prefab_name](const PrefabDefinition& prefab) {
        return prefab_name == prefab.name;
    };

    const auto it = std::find_if(prefabs.begin(), prefabs.end(), find_by_name);
    if(it != prefabs.end())
        return &(*it);

    return nullptr;
}
