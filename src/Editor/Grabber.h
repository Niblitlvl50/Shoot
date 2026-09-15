
#pragma once

#include "Math/Vector.h"
#include "Rendering/Color.h"
#include <functional>

namespace editor
{
    struct Grabber
    {
        math::Vector position;
        bool hoover = false;
        mono::Color::RGBA color = { 1.0f, 0.5f, 0.0f, 0.8f };
        std::function<void (const math::Vector&)> callback;
    };
}
