
#pragma once

#include "MonoFwd.h"
#include "MonoPtrFwd.h"
#include <functional>

namespace game
{
    using DestroyedFunction = std::function<void (unsigned int rectord_id)>;
    
    struct SpawnPhysicsEntityEvent
    {
        SpawnPhysicsEntityEvent(const mono::IPhysicsEntityPtr& entity, int layer, DestroyedFunction destroyed_func)
            : entity(entity),
              layer(layer),
              destroyed_func(destroyed_func)
        { }

        mono::IPhysicsEntityPtr entity;
        const int layer;
        const DestroyedFunction destroyed_func;
    };
}
