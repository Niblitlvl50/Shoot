
#pragma once

#include "MonoFwd.h"

namespace game
{
    struct SpawnEntityEvent
    {
        SpawnEntityEvent(const mono::IEntityPtr& entity, int layer)
            : entity(entity)
            , layer(layer)
        { }

        const mono::IEntityPtr entity;
        const int layer;
    };
}

