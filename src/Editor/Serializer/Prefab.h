
#pragma once

#include "Math/Vector.h"
#include <string>

namespace editor
{
    struct PrefabData
    {
        std::string name;
        math::Vector position;
        float rotation = 0.0f;
    };
}
