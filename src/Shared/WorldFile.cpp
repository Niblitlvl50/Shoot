
#include "WorldFile.h"
#include "Math/Serialize.h"

#include "Entity/IEntityManager.h"
#include "Component.h"
#include "Serialize.h"

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

bool world::WriteWorld(file::FilePtr& file, const LevelFileHeader& level)
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

    const int n_bounds_vertices = static_cast<int>(level.bounds.size());
    WriteValues(&n_bounds_vertices, file.get());
    WriteValues(level.bounds.data(), file.get(), n_bounds_vertices);

    return true;
}

bool world::ReadWorld(const file::FilePtr& file, LevelFileHeader& level)
{
    std::vector<byte> bytes;
    file::FileRead(file, bytes);

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

    if(level.version < 3)
        return true;

    int n_bounds_vertices = 0;
    ReadValues(&n_bounds_vertices, bytes, offset);

    level.bounds.resize(n_bounds_vertices);
    ReadValues(level.bounds.data(), bytes, offset, n_bounds_vertices);

    return true;
}

bool world::WriteWorldObjectsBinary(file::FilePtr& file, const world::WorldObjectsHeader& objects_header)
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

bool world::ReadWorldObjectsBinary(const file::FilePtr& file, world::WorldObjectsHeader& objects)
{
    if(!file)
        return false;

    std::vector<byte> bytes;
    file::FileRead(file, bytes);

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

std::vector<uint32_t> world::ReadWorldComponentObjects(const char* file_name, IEntityManager* entity_manager)
{
    std::vector<uint32_t> created_entities;

    file::FilePtr file = file::OpenAsciiFile(file_name);
    if(!file)
        return created_entities;

    std::vector<byte> file_data;
    file::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& entities = json["entities"];

    for(const auto& json_entity : entities)
    {
        const std::string& entity_name = json_entity["name"];
        mono::Entity new_entity = entity_manager->CreateEntity(entity_name.c_str(), std::vector<uint32_t>());

        for(const auto& json_component : json_entity["components"])
        {
            Component component;
            component.name = json_component["name"];
            component.hash = mono::Hash(component.name.c_str());

            for(const Attribute& property : json_component["properties"])
                component.properties.push_back(property);

            const Component& default_component = DefaultComponentFromHash(component.hash);
            world::UnionAttributes(component.properties, default_component.properties);

            entity_manager->AddComponent(new_entity.id, component.hash);
            entity_manager->SetComponentData(new_entity.id, component.hash, component.properties);
        }

        created_entities.push_back(new_entity.id);
    }

    return created_entities;
}
