
#pragma once

#include "MonoFwd.h"
#include <cstdint>

namespace mono
{
    struct RenderInitParams;
}

namespace game
{
    struct Config;

    void CreateGameSystems(
        uint32_t max_entities,
        mono::SystemContext& system_context,
        mono::EventHandler& event_handler,
        mono::ICamera& camera,
        const mono::RenderInitParams& render_params,
        const game::Config& game_config);
}
