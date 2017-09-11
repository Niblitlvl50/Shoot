
#pragma once

#include "ObjectProxies/IObjectProxy.h"

#include <vector>
#include <memory>

namespace editor
{
    class PolygonEntity;
    class PathEntity;
    class SpriteEntity;
    class Prefab;
    
    class IObjectProxy;

    class EntityRepository;
    class ObjectFactory;

    std::vector<IObjectProxyPtr> LoadPolygons(const char* file_name, const editor::ObjectFactory& factory);
    void SavePolygons(const char* file_name, const std::vector<IObjectProxy*>& polygons);

    std::vector<IObjectProxyPtr> LoadPaths(const char* file_name, const editor::ObjectFactory& factory);
    void SavePaths(const char* file_name, const std::vector<IObjectProxy*>& paths);

    std::vector<IObjectProxyPtr> LoadObjects(const char* file_name, const editor::ObjectFactory& factory);
    void SaveObjects(const char* file_name, const std::vector<std::shared_ptr<editor::SpriteEntity>>& objects);

    std::vector<IObjectProxyPtr> LoadObjects2(const char* file_name, const editor::ObjectFactory& factory);
    void SaveObjects2(const char* file_name, const std::vector<IObjectProxy*>& proxies);

    std::vector<IObjectProxyPtr> LoadPrefabs(const char* file_name, const editor::ObjectFactory& factory);
    void SavePrefabs(const char* file_name, const std::vector<IObjectProxy*>& prefabs);
}
