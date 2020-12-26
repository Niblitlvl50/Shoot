
#pragma once

#include "ObjectProxies/IObjectProxy.h"
#include "WorldFile.h"
#include <vector>

namespace mono
{
    class TransformSystem;
}

namespace editor
{
    class ObjectFactory;
    class Editor;

    std::vector<IObjectProxyPtr> LoadPaths(const char* file_name, const editor::ObjectFactory& factory);
    std::vector<IObjectProxyPtr> LoadComponentObjects(const char* file_name, mono::IEntityManager* entity_manager, mono::TransformSystem* transform_system, Editor* editor);

    struct World
    {
        shared::LevelData leveldata;
        std::vector<IObjectProxyPtr> loaded_proxies;
    };

    World LoadWorld(
        const char* file_name,
        const editor::ObjectFactory& factory,
        mono::IEntityManager* entity_manager,
        mono::TransformSystem* transform_system,
        Editor* editor);

    void SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies, const shared::LevelMetadata& level_data);
}
