
#pragma once

#include <MonoFwd.h>
#include <cstdint>

namespace game
{
    struct Config;
    
    struct ZoneCreationContext
    {
        uint32_t num_entities = 0;
        mono::EventHandler* event_handler = nullptr;
        mono::SystemContext* system_context = nullptr;
        game::Config* game_config = nullptr;
        const char* zone_filename = nullptr;
    };
}
