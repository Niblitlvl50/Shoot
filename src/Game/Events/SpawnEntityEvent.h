
#pragma once

#include "MonoFwd.h"
#include <functional>

namespace game
{
    using DestroyedFunction = std::function<void (unsigned int rectord_id)>;
    
    struct SpawnEntityEvent
    {
        SpawnEntityEvent(const mono::IEntityPtr& entity, int layer, DestroyedFunction destroyed_func)
            : entity(entity),
              layer(layer),
              destroyed_func(destroyed_func)
        { }

        const mono::IEntityPtr entity;
        const int layer;
        const DestroyedFunction destroyed_func;
    };
}

