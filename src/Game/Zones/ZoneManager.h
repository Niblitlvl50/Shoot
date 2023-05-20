
#pragma once

#include "ZoneCreationContext.h"
#include "MonoFwd.h"

namespace game
{

    class ZoneManager
    {
    public:
        static void Run(
            mono::ICamera* camera, ZoneCreationContext zone_context, const char* initial_zone_name = nullptr);
    };
}
