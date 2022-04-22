
#pragma once

#include "IUpdatable.h"
#include <cstdint>

namespace game
{
    class IEntityLogic : public mono::IUpdatable
    {
    public:

        virtual void DrawDebugInfo(class IDebugDrawer* debug_drawer) const
        { }
    };
}
