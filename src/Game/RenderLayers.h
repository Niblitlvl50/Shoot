
#pragma once

namespace game
{
    enum LayerId
    {
        BACKGROUND,
        PRE_GAMEOBJECTS,
        GAMEOBJECTS,
        POST_GAMEOBJECTS,

        // Layers not affected by lighting.
        GAMEOBJECTS_UI,
        UI,
        UI_OVERLAY,
        UI_DEBUG
    };
}
