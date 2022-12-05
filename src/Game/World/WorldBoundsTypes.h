
#pragma once

namespace game
{
    enum class PolygonDrawLayer : int
    {
        PRE_GAMEOBJECTS,
        POST_GAMEOBJECTS
    };

    constexpr const char* g_polygon_draw_layer_strings[] = {
        "Pre GameObjects",
        "Post GameObjects",
    };

    inline const char* PolygonDrawLayerToString(PolygonDrawLayer draw_layer)
    {
        return g_polygon_draw_layer_strings[static_cast<int>(draw_layer)];
    }
}
