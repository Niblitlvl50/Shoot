
#include "WorldFile.h"
#include "Math/Serialize.h"

#include "nlohmann/json.hpp"
#include <cstdio>

namespace
{
    template <typename T>
    void ReadValues(T* out_data, const std::vector<byte>& bytes, int& offset, size_t n_values = 1)
    {
        std::memcpy(out_data, bytes.data() + offset, sizeof(T) * n_values);
        offset += (sizeof(T) * n_values);
    }

    template <typename T>
    void WriteValues(T* data, FILE* file, size_t n_values = 1)
    {
        std::fwrite(data, sizeof(T), n_values, file);
    }
}

bool world::WriteWorld(File::FilePtr& file, const LevelFileHeader& level)
{
    WriteValues(&level.version, file.get());

    const int n_polygons = static_cast<int>(level.polygons.size());
    WriteValues(&n_polygons, file.get());

    for(int index = 0; index < n_polygons; ++index)
    {
        const world::PolygonData& polygon = level.polygons[index];
        const int n_vertices = static_cast<int>(polygon.vertices.size());

        WriteValues(&polygon.position,          file.get());
        WriteValues(&polygon.local_offset,      file.get());
        WriteValues(&polygon.rotation,          file.get());
        WriteValues(&polygon.texture,           file.get());
        WriteValues(&n_vertices,                file.get());
        WriteValues(polygon.vertices.data(),    file.get(), n_vertices);
    }

    const int n_prefabs = static_cast<int>(level.prefabs.size());
    WriteValues(&n_prefabs, file.get());

    for(int index = 0; index < n_prefabs; ++index)
    {
        const world::PrefabData& prefab = level.prefabs[index];

        WriteValues(&prefab.name,       file.get());
        WriteValues(&prefab.position,   file.get());
        WriteValues(&prefab.scale,      file.get());
        WriteValues(&prefab.rotation,   file.get());
    }

    const int n_bounds_vertices = static_cast<int>(level.bounds.size());
    WriteValues(&n_bounds_vertices, file.get());
    WriteValues(level.bounds.data(), file.get(), n_bounds_vertices);

    return true;
}

bool world::ReadWorld(const File::FilePtr& file, LevelFileHeader& level)
{
    std::vector<byte> bytes;
    File::FileRead(file, bytes);

    int offset = 0;
    int n_polygons = 0;

    ReadValues(&level.version,  bytes, offset);
    ReadValues(&n_polygons,     bytes, offset);

    level.polygons.resize(n_polygons);

    for(int index = 0; index < n_polygons; ++index)
    {
        world::PolygonData& polygon = level.polygons[index];

        ReadValues(&polygon.position,       bytes, offset);
        ReadValues(&polygon.local_offset,   bytes, offset);
        ReadValues(&polygon.rotation,       bytes, offset);
        ReadValues(&polygon.texture,        bytes, offset);

        int n_vertices = 0;
        ReadValues(&n_vertices, bytes, offset);
        polygon.vertices.resize(n_vertices);

        ReadValues(polygon.vertices.data(), bytes, offset, n_vertices);
    }

    if(level.version < 2)
        return true;

    int n_prefabs = 0;
    ReadValues(&n_prefabs, bytes, offset);

    level.prefabs.resize(n_prefabs);

    for(int index = 0; index < n_prefabs; ++index)
    {
        world::PrefabData& prefab = level.prefabs[index];

        ReadValues(&prefab.name,        bytes, offset);
        ReadValues(&prefab.position,    bytes, offset);
        ReadValues(&prefab.scale,       bytes, offset);
        ReadValues(&prefab.rotation,    bytes, offset);
    }

    if(level.version < 3)
        return true;

    int n_bounds_vertices = 0;
    ReadValues(&n_bounds_vertices, bytes, offset);

    level.bounds.resize(n_bounds_vertices);
    ReadValues(level.bounds.data(), bytes, offset, n_bounds_vertices);

    return true;
}

bool world::WriteWorldObjectsBinary(File::FilePtr& file, const world::WorldObjectsHeader& objects_header)
{
    WriteValues(&objects_header.version, file.get());

    const int n_objects = static_cast<int>(objects_header.objects.size());
    WriteValues(&n_objects, file.get());

    for(int index = 0; index < n_objects; ++index)
    {
        const world::WorldObject& world_object = objects_header.objects[index];
        const int n_attributes = static_cast<int>(world_object.attributes.size());

        WriteValues(&world_object.name, file.get());
        WriteValues(&n_attributes, file.get());
        WriteValues(world_object.attributes.data(), file.get(), n_attributes);
    }

    return true;
}

bool world::ReadWorldObjectsBinary(const File::FilePtr& file, world::WorldObjectsHeader& objects)
{
    if(!file)
        return false;

    std::vector<byte> bytes;
    File::FileRead(file, bytes);

    if(bytes.empty())
        return false;

    int offset = 0;
    int n_objects = 0;

    ReadValues(&objects.version,    bytes, offset);
    ReadValues(&n_objects,          bytes, offset);

    objects.objects.resize(n_objects);

    for(int index = 0; index < n_objects; ++index)
    {
        world::WorldObject& world_object = objects.objects[index];
        
        ReadValues(&world_object.name, bytes, offset);

        int n_attributes = 0;
        ReadValues(&n_attributes, bytes, offset);
        world_object.attributes.resize(n_attributes);
        ReadValues(world_object.attributes.data(), bytes, offset, n_attributes);
    }

    return true;
}


bool world::WriteWorldObjects(File::FilePtr& file, const std::vector<WorldObject>& objects)
{
    nlohmann::json json_object_collection;
    
    for(const auto& object : objects)
    {
        nlohmann::json json_object;

        json_object["name"] = object.name;
        nlohmann::json& attributes = json_object["attributes"];

        for(auto& attribute : object.attributes)
        {
            nlohmann::json json_attribute;
            json_attribute["id"] = attribute.id;
            json_attribute["type"] = static_cast<int>(attribute.attribute.type);
            json_attribute["data"] = attribute.attribute.string_value;
            attributes.push_back(json_attribute);
        }

        json_object_collection.push_back(json_object);
    }

    nlohmann::json json;
    json["objects"] = json_object_collection;

    const std::string& serialized_json = json.dump(4);
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
    
    return true;
}

bool world::ReadWorldObjects(const File::FilePtr& file, std::vector<WorldObject>& objects)
{    
    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& json_objects = json["objects"];

    for(const auto& json_object : json_objects)
    {
        const std::string& object_name = json_object["name"];

        WorldObject object;
        std::memset(object.name, 0, WorldObjectNameMaxLength);
        std::memcpy(object.name, object_name.data(), WorldObjectNameMaxLength -1);

        for(auto& json_attribute : json_object["attributes"])
        {
            Attribute attribute;
            attribute.id = json_attribute["id"];
                        
            const std::string data = json_attribute["data"];
            attribute.attribute = data.c_str();
            
            const int type = static_cast<int>(json_attribute["type"]);
            attribute.attribute.type = Variant::Type(type);
            
            object.attributes.push_back(attribute);
        }

        objects.emplace_back(object);
    }

    return true;
}
