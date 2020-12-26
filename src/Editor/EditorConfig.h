
#pragma once

#include "Math/Vector.h"
#include "Rendering/Color.h"

#include <string>

namespace editor
{
    struct Config
    {
        math::Vector camera_position;
        math::Vector camera_viewport = math::Vector(600.0f, 400.0f);
        math::Vector window_position;
        math::Vector window_size = math::Vector(1200.0f, 800.0f);

        bool draw_object_names = false;
        bool draw_snappers = false;
        bool draw_outline = false;
        bool draw_metadata = false;
        mono::Color::RGBA background_color = mono::Color::RGBA(0.07f, 0.07f, 0.07f, 1.0f);

        bool snap_to_grid = false;
        math::Vector grid_size = math::Vector(1.0f, 1.0f);

        std::string selected_world;
    };

    bool SaveConfig(const char* config_file, const Config& config);
    bool LoadConfig(const char* config_file, Config& config);
}
