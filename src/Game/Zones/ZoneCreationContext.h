
#pragma once

#include <MonoFwd.h>

namespace game
{
    class DamageController;

    struct ZoneCreationContext
    {
        mono::EventHandler* event_handler = nullptr;
        game::DamageController* damage_controller = nullptr;
    };
}
