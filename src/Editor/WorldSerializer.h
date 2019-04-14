
#pragma once

#include "ObjectProxies/IObjectProxy.h"
#include <vector>

class IEntityManager;

namespace mono
{
    class TransformSystem;
}

namespace editor
{
    class ObjectFactory;

    std::vector<IObjectProxyPtr> LoadPolygons(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadPaths(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadComponentObjects(const char* file_name, IEntityManager* entity_manager, mono::TransformSystem* transform_system);
    std::vector<IObjectProxyPtr> LoadObjectsBinary(const char* file_name, const editor::ObjectFactory& factory);

    std::vector<IObjectProxyPtr> LoadWorld(
        const char* file_name, const editor::ObjectFactory& factory, IEntityManager* entity_manager, mono::TransformSystem* transform_system);
    void SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies);
}
