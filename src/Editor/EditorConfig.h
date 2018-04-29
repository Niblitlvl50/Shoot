
#pragma once

#include "Math/Vector.h"
#include "Math/Quad.h"
#include "Rendering/Color.h"

namespace editor
{
    struct Config
    {
        math::Vector camera_position;
        math::Quad camera_viewport;
        math::Vector window_position;
        math::Vector window_size;

        bool draw_object_names;
        mono::Color::RGBA background_color;
    };

    bool SaveConfig(const char* config_file, const Config& config);
    bool LoadConfig(const char* config_file, Config& config);
}
