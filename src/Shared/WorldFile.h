
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Rendering/Color.h"

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

struct Component;

namespace shared
{
    struct LevelMetadata
    {
        math::Vector camera_position;
        math::Vector camera_size;
        math::Vector player_spawn_point;
        mono::Color::RGBA background_color;
        mono::Color::RGBA ambient_shade;

        math::Vector background_size;
        std::string background_texture;

        std::vector<std::string> triggers;
        std::vector<std::string> conditions;

        math::Vector navmesh_start;
        math::Vector navmesh_end;
        float navmesh_density;
    };

    struct LevelData
    {
        LevelMetadata metadata;
        std::vector<uint32_t> loaded_entities;
    };

    using EntityCreationCallback
        = std::function<void (const mono::Entity& entity, const std::string& folder, const std::vector<Component>& components)>;

    LevelData ReadWorldComponentObjects(
        const char* filename, mono::IEntityManager* entity_manager, EntityCreationCallback creation_callback);

    using ComponentFilterCallback = std::function<bool (uint32_t component_hash)>;

    LevelData ReadWorldComponentObjectsFiltered(
        const char* filename, mono::IEntityManager* entity_manager, ComponentFilterCallback component_filter);
}
