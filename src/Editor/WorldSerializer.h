
#pragma once

#include "ObjectProxies/IObjectProxy.h"

#include <vector>
#include <memory>

namespace editor
{
    class ObjectFactory;

    std::vector<IObjectProxyPtr> LoadPolygons(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadPaths(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadObjects(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadObjectsBinary(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadPrefabs(const char* file_name, const editor::ObjectFactory& factory);

    std::vector<IObjectProxyPtr> LoadWorld(const char* file_name, const editor::ObjectFactory& factory);    
    void SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies);
}
