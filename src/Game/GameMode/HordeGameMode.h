
#pragma once

#include "IGameMode.h"

namespace game
{
    class HordeGameMode : public IGameMode
    {
    public:

        void Begin(
            mono::IZone* zone,
            mono::IRenderer* renderer,
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            const struct LevelMetadata& level_metadata) override;
        int End(mono::IZone* zone) override;
        void Update(const mono::UpdateContext& update_context) override;
    };
}
