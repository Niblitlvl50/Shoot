
#pragma once

#include "Rendering/RenderFwd.h"

namespace game
{
    struct AddDrawableEvent
    {
        AddDrawableEvent(const mono::IDrawable* drawable)
            : drawable(drawable)
        { }

        const mono::IDrawable* drawable;
    };

    struct RemoveDrawableEvent
    {
        RemoveDrawableEvent(const mono::IDrawable* drawable)
            : drawable(drawable)
        { }

        const mono::IDrawable* drawable;
    };
}
