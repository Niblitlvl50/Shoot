
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Rendering/Color.h"

#include <vector>
#include <string>
#include <cstdint>
#include <functional>

struct Component;

namespace game
{
    struct LevelMetadata
    {
        std::string level_name;
        std::string level_description;

        std::string level_game_mode;

        math::Vector camera_position;
        math::Vector camera_size;
        math::Vector player_spawn_point;

        bool spawn_package = true;
        bool use_package_spawn_position = false;
        math::Vector package_spawn_position;

        mono::Color::RGBA background_color;
        mono::Color::RGBA ambient_shade;

        math::Vector background_size;
        std::string background_texture;

        std::string background_music;

        std::vector<std::string> triggers;

        math::Vector navmesh_start;
        math::Vector navmesh_end;
        float navmesh_density = 1.0f;

        int time_limit_s = 0;

        std::string completed_trigger;
        std::string failed_trigger;
    };

    struct LevelData
    {
        LevelMetadata metadata;
        std::vector<uint32_t> loaded_entities;
    };

    using EntityCreationCallback = std::function<
        void (const mono::Entity& entity, const std::vector<Component>& components)>;

    LevelData ReadWorldComponentObjects(
        const char* filename, mono::IEntityManager* entity_manager, EntityCreationCallback creation_callback);
}
