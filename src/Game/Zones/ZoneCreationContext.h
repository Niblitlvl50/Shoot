
#pragma once

#include <MonoFwd.h>

namespace game
{
    struct Config;
    
    struct ZoneCreationContext
    {
        mono::EventHandler* event_handler = nullptr;
        mono::SystemContext* system_context = nullptr;
        game::Config* game_config = nullptr;
    };
}
