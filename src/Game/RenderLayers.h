
#pragma once

namespace game
{
    enum LayerId
    {
        BACKGROUND,
        PRE_GAMEOBJECTS,
        GAMEOBJECTS,
        POST_GAMEOBJECTS,
        GAMEOBJECTS_UI,

        // Layers not affected by lighting.
        UI,
        UI_OVERLAY,
        UI_DEBUG
    };
}
