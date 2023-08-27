
#pragma once

#include "Math/Vector.h"
#include <vector>

namespace game
{
    enum NavigationDebugComponents
    {
        NDC_NONE = 0,
        DRAW_NAVMESH = 1,
        DRAW_RECENT_PATHS = 2,
    };

    constexpr uint32_t all_navigation_debug_component[] = {
        NavigationDebugComponents::DRAW_NAVMESH,
        NavigationDebugComponents::DRAW_RECENT_PATHS,
    };

    inline const char* NavigationDebugComponentToString(uint32_t debug_component)
    {
        switch(debug_component)
        {
        case NDC_NONE:
            return "None";
        case DRAW_NAVMESH:
            return "Navmesh";
        case DRAW_RECENT_PATHS:
            return "Recent Paths";
        }

        return "Unknown";
    }
    
    struct NavmeshNode
    {
        int data_index;
        int neighbours_index[8];
    };

    struct NavmeshContext
    {
        std::vector<math::Vector> points;
        std::vector<NavmeshNode> nodes;
    };
}
