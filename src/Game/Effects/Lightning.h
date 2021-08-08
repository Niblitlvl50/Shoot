
#pragma once

#include "Math/Vector.h"
#include <vector>
#include <cstdint>

namespace game
{
    std::vector<math::Vector> GenerateLightning(const math::Vector& from, const math::Vector& to, uint32_t points_per_meter);
}
