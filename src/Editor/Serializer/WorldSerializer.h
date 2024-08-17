
#pragma once

#include "MonoFwd.h"
#include "ObjectProxies/IObjectProxy.h"
#include "WorldFile.h"
#include <vector>
#include <string>

namespace editor
{
    struct World
    {
        game::LevelData leveldata;
        std::vector<IObjectProxyPtr> loaded_proxies;
    };

    World LoadWorld(
        const char* file_name,
        mono::IEntityManager* entity_manager,
        mono::TransformSystem* transform_system,
        class Editor* editor);

    void SaveWorld(
        const char* file_name, const std::vector<IObjectProxy*>& proxies, const game::LevelMetadata& level_data);

    void WriteComponentEntities(
        const std::string& file_path,
        const game::LevelMetadata& level_metadata,
        const std::vector<IObjectProxy*>& proxies);
}
