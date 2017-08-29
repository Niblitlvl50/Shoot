
#include "WorldFile.h"
#include "Math/Serialize.h"

#include "nlohmann_json/json.hpp"
#include <cstdio>


bool world::WriteWorld(File::FilePtr& file, const LevelFileHeader& level)
{
    std::fwrite(&level.version, sizeof(int), 1, file.get());

    const int n_polygons = static_cast<int>(level.polygons.size());
    std::fwrite(&n_polygons, sizeof(int), 1, file.get());

    for(int index = 0; index < n_polygons; ++index)
    {
        const world::PolygonData& polygon = level.polygons[index];
        const int n_vertices = static_cast<int>(polygon.vertices.size());

        std::fwrite(&polygon.position,          sizeof(math::Vector), 1,          file.get());
        std::fwrite(&polygon.local_offset,      sizeof(math::Vector), 1,          file.get());
        std::fwrite(&polygon.rotation,          sizeof(float),        1,          file.get());
        std::fwrite(&polygon.texture_repeate,   sizeof(float),        1,          file.get());
        std::fwrite(&polygon.texture,           sizeof(char),        64,          file.get());
        std::fwrite(&n_vertices,                sizeof(int),          1,          file.get());
        std::fwrite(polygon.vertices.data(),    sizeof(math::Vector), n_vertices, file.get());
    }

    return true;
}

bool world::ReadWorld(File::FilePtr& file, LevelFileHeader& level)
{
    std::vector<byte> bytes;
    File::FileRead(file, bytes);

    int offset = 0;

    std::memcpy(&level.version,     bytes.data() + offset, sizeof(int));
    offset += sizeof(int);

    int n_polygons = 0;
    std::memcpy(&n_polygons,        bytes.data() + offset, sizeof(int));
    offset += sizeof(int);

    level.polygons.resize(n_polygons);

    for(int index = 0; index < n_polygons; ++index)
    {
        world::PolygonData& polygon = level.polygons[index];

        std::memcpy(&polygon.position, bytes.data() + offset, sizeof(math::Vector));
        offset += sizeof(math::Vector);

        std::memcpy(&polygon.local_offset, bytes.data() + offset, sizeof(math::Vector));
        offset += sizeof(math::Vector);

        std::memcpy(&polygon.rotation, bytes.data() + offset, sizeof(float));
        offset += sizeof(float);

        std::memcpy(&polygon.texture_repeate, bytes.data() + offset, sizeof(float));
        offset += sizeof(float);

        std::memcpy(&polygon.texture, bytes.data() + offset, sizeof(char) * 64);
        offset += sizeof(char) * 64;

        int n_vertices = 0;
        std::memcpy(&n_vertices,        bytes.data() + offset, sizeof(int));
        offset += sizeof(int);

        polygon.vertices.resize(n_vertices);

        std::memcpy(polygon.vertices.data(), bytes.data() + offset, sizeof(math::Vector) * n_vertices);
        offset += sizeof(math::Vector) * n_vertices;
    }

    return true;
}

bool world::WriteWorldObjects(File::FilePtr& file, const std::vector<WorldObject>& objects)
{
    nlohmann::json json_object_collection;
    
    for(auto& object : objects)
    {
        nlohmann::json json_object;
        json_object["name"] = object.name;
        json_object["position"] = object.position;
        json_object["rotation"] = object.rotation;

        json_object_collection.push_back(json_object);
    }

    nlohmann::json json;
    json["objects"] = json_object_collection;

    const std::string& serialized_json = json.dump(4);
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
    
    return true;
}

bool world::ReadWorldObjects(File::FilePtr& file, std::vector<WorldObject>& objects)
{    
    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& json_objects = json["objects"];

    for(const auto& json_object : json_objects)
    {
        WorldObject object;

        object.name = json_object["name"];
        object.position = json_object["position"];
        object.rotation = json_object["rotation"];

        objects.emplace_back(object);
    }

    return true;
}
