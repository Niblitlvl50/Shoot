
#pragma once

#include "MonoFwd.h"
#include "ObjectProxies/IObjectProxy.h"
#include "WorldFile.h"
#include <vector>

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

    void SaveWorld(const char* file_name, const std::vector<IObjectProxyPtr>& proxies, const game::LevelMetadata& level_data);
}
