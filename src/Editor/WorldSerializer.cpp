
#include "WorldSerializer.h"
#include "Objects/Polygon.h"
#include "Objects/Path.h"
#include "Objects/SpriteEntity.h"
#include "Objects/Prefab.h"
#include "Math/Matrix.h"
#include "Math/Serialize.h"
#include "System/File.h"
#include "WorldFile.h"
#include "EntityRepository.h"

#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

#include "ObjectProxies/IObjectProxy.h"

#include "nlohmann_json/json.hpp"

#include "DefinedAttributes.h"
#include "ObjectFactory.h"

#include "Serializer/JsonSerializer.h"
#include "Serializer/BinarySerializer.h"

#include <algorithm>

std::vector<IObjectProxyPtr> editor::LoadPolygons(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> polygon_data;

    if(!file_name)
        return polygon_data;

    File::FilePtr file = File::OpenBinaryFile(file_name);
    if(!file)
        return polygon_data;

    world::LevelFileHeader level_data;
    world::ReadWorld(file, level_data);

    polygon_data.reserve(level_data.polygons.size());

    for(const world::PolygonData& polygon : level_data.polygons)
    {
        if(polygon.vertices.empty())
            continue;

        auto proxy = factory.CreatePolygon(
            polygon.position, polygon.local_offset, polygon.rotation, polygon.vertices, polygon.texture);
        polygon_data.push_back(std::move(proxy));
    }

    return polygon_data;
}

void editor::SavePolygons(const char* file_name, const std::vector<IObjectProxy*>& polygons)
{
    if(!file_name)
        return;

    BinarySerializer serializer;

    for(auto* proxy : polygons)
        proxy->Visit(serializer);

    serializer.WritePolygonFile(file_name);
}

std::vector<IObjectProxyPtr> editor::LoadPaths(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> paths;

    File::FilePtr file = File::OpenAsciiFile(file_name);
    if(!file)
        return paths;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const std::vector<std::string>& path_names = json["path_files"];

    paths.reserve(path_names.size());

    const auto get_name = [](const std::string& file_name) {
        const size_t slash_pos = file_name.find_last_of('/');
        const size_t dot_pos = file_name.find_last_of('.');
        return file_name.substr(slash_pos +1, dot_pos - slash_pos -1);
    };

    for(const std::string& file : path_names)
    {
        std::shared_ptr<mono::IPath> path = mono::CreatePath(file.c_str());
        auto proxy = factory.CreatePath(get_name(file), path->GetPathPoints());
        proxy->Entity()->SetPosition(path->GetGlobalPosition());

        paths.push_back(std::move(proxy));
    }

    return paths;
}

void editor::SavePaths(const char* file_name, const std::vector<editor::IObjectProxy*>& paths)
{
    JsonSerializer serializer(file_name);

    for(auto* path : paths)
        path->Visit(serializer);

    serializer.WritePathFile();
}

void editor::SaveObjects(const char* file_name, const std::vector<std::shared_ptr<editor::SpriteEntity>>& objects)
{
    nlohmann::json json_object_collection;

    for(auto& object : objects)
    {
        nlohmann::json json_object;
        json_object["name"] = object->Name();
        json_object["position"] = object->Position();
        json_object["rotation"] = object->Rotation();

        json_object_collection.push_back(json_object);
    }

    nlohmann::json json;
    json["objects"] = json_object_collection;

    const std::string& serialized_json = json.dump(4);

    File::FilePtr file = File::CreateAsciiFile(file_name);
    std::fwrite(serialized_json.data(), serialized_json.length(), sizeof(char), file.get());
}

void editor::SaveObjects2(const char* file_name, const std::vector<IObjectProxy*>& proxies)
{
    File::FilePtr file = File::CreateBinaryFile(file_name);
    if(!file)
        return;
    
    world::WorldObjectsHeader world_header;
    
    for(auto proxy : proxies)
    {
        world::WorldObject world_object;
        
        const char* name = proxy->Name();
        const size_t length = std::strlen(name);
        const size_t data_length = std::min(length, size_t(24) -1);
    
        std::memcpy(world_object.name, name, data_length);
        world_object.name[data_length] = '\0';
        world_object.attributes = proxy->GetAttributes();

        world_header.objects.push_back(world_object);
    }

    world::WriteWorldObjects2(file, world_header);
}

std::vector<IObjectProxyPtr> editor::LoadObjects2(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> objects;

    File::FilePtr file = File::OpenBinaryFile(file_name);
    if(!file)
        return objects;

    world::WorldObjectsHeader world_header;
    world::ReadWorldObjects2(file, world_header);

    for(auto& world_object : world_header.objects)
    {
        IObjectProxyPtr proxy = factory.CreateObject(world_object.name);
        proxy->SetAttributes(world_object.attributes);
        
        objects.push_back(std::move(proxy));
    }

    return objects;
}


std::vector<IObjectProxyPtr> editor::LoadObjects(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> objects;
    
    File::FilePtr file = File::OpenAsciiFile(file_name);
    if(!file)
        return objects;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& json_objects = json["objects"];

    for(const auto& json_object : json_objects)
    {
        const std::string& name = json_object["name"];
        const math::Vector& position = json_object["position"];
        const float rotation = json_object["rotation"];

        const std::vector<ID_Attribute> attributes = {
            { world::POSITION_ATTRIBUTE, ObjectAttribute(position) },
            { world::RADIUS_ATTRIBUTE,   ObjectAttribute(rotation) }
        };

        IObjectProxyPtr proxy = factory.CreateObject(name.c_str());
        proxy->SetAttributes(attributes);
        
        objects.push_back(std::move(proxy));
    }

    return objects;
}

std::vector<IObjectProxyPtr> editor::LoadPrefabs(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> prefabs;

    File::FilePtr file = File::OpenAsciiFile(file_name);
    if(!file)
        return prefabs;

    std::vector<byte> file_data;
    File::FileRead(file, file_data);

    const nlohmann::json& json = nlohmann::json::parse(file_data);
    const nlohmann::json& json_prefabs = json["prefabs"];

    for(const auto& json_prefab : json_prefabs)
    {
        const std::string& name = json_prefab["name"];
        const math::Vector& position = json_prefab["position"];
        const float rotation = json_prefab["rotation"];

        IObjectProxyPtr proxy = factory.CreatePrefab(name);
        auto prefab = proxy->Entity();

        prefab->SetPosition(position);
        prefab->SetRotation(rotation);
    
        prefabs.push_back(std::move(proxy));
    }

    return prefabs;
}

void editor::SavePrefabs(const char* file_name, const std::vector<IObjectProxy*>& prefabs)
{

}
