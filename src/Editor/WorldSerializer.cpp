
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

    const auto check_for_nan = [](const math::Vector& vertex) {
        return std::isnan(vertex.x) || std::isnan(vertex.y);
    };

    for(const world::PolygonData& polygon : level_data.polygons)
    {
        if(polygon.vertices.empty())
            continue;

        if(std::any_of(polygon.vertices.begin(), polygon.vertices.end(), check_for_nan))
            continue;

        auto proxy = factory.CreatePolygon(
            polygon.position, polygon.local_offset, polygon.rotation, polygon.vertices, polygon.texture);
        polygon_data.push_back(std::move(proxy));
    }

    return polygon_data;
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

std::vector<IObjectProxyPtr> editor::LoadObjectsBinary(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> objects;

    File::FilePtr file = File::OpenBinaryFile(file_name);
    if(!file)
        return objects;

    world::WorldObjectsHeader world_header;
    world::ReadWorldObjectsBinary(file, world_header);

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

    for(const auto& json_object : json["objects"])
    {
        const std::string& name = json_object["name"];

        std::vector<Attribute> attributes;

        for(auto&& json_attribute : json_object["attributes"])
        {
            const std::string& attribute_name = json_attribute["name"];

            Attribute attribute;
            attribute.id = mono::Hash(attribute_name.c_str());
            attribute.attribute = world::DefaultAttributeFromHash(attribute.id);

            switch(attribute.attribute.type)
            {
                case Variant::Type::INT:
                    attribute.attribute = (int)json_attribute["value"];
                    break;
                case Variant::Type::FLOAT:
                    attribute.attribute = (float)json_attribute["value"];
                    break;
                case Variant::Type::STRING:
                {
                    const std::string& string_value = json_attribute["value"];
                    attribute.attribute = string_value.c_str();
                    break;
                }
                case Variant::Type::POINT:
                    attribute.attribute = (math::Vector)json_attribute["value"];
                    break;
                case Variant::Type::NONE:
                    break;
            }

            attributes.push_back(std::move(attribute));
        }

        IObjectProxyPtr proxy = factory.CreateObject(name.c_str());
        if(proxy)
        {
            proxy->SetAttributes(attributes);        
            objects.push_back(std::move(proxy));
        }
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
        if(proxy)
        {
            auto prefab = proxy->Entity();
            prefab->SetPosition(position);
            prefab->SetRotation(rotation);
        
            prefabs.push_back(std::move(proxy));
        }
    }

    return prefabs;
}

std::vector<IObjectProxyPtr> editor::LoadWorld(const char* file_name, const editor::ObjectFactory& factory)
{
    std::vector<IObjectProxyPtr> world_objects;

    //auto objects_bin = LoadObjectsBinary("res/world.objects.bin", factory);
    auto objects = LoadObjects("res/world.objects", factory);
    auto paths = LoadPaths("res/world.paths", factory);
    auto prefabs = LoadPrefabs("res/world.prefabs", factory);
    auto polygons = LoadPolygons(file_name, factory);

    for(auto& proxy : objects)
        world_objects.push_back(std::move(proxy));
    
    for(auto& proxy : prefabs)
        world_objects.push_back(std::move(proxy));

    for(auto& proxy : paths)
        world_objects.push_back(std::move(proxy));

    for(auto& proxy : polygons)
        world_objects.push_back(std::move(proxy));

    return world_objects;    
}

void editor::SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies)
{
    {
        BinarySerializer serializer;
        
        for(auto& proxy : proxies)
            proxy->Visit(serializer);
        
        serializer.WritePolygonFile(file_name);
        serializer.WriteObjects("res/world.objects.bin");
    }

    {
        JsonSerializer serializer;
        
        for(auto& proxy : proxies)
            proxy->Visit(serializer);
    
        serializer.WriteEntities("res/world.objects");
        serializer.WritePathFile("res/world.paths");
        serializer.WritePrefabs("res/world.prefabs");
    }
}
