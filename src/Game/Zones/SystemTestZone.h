
#pragma once

#include "GameZone.h"
#include "ZoneCreationContext.h"
#include "GameConfig.h"

#include <memory>

namespace game
{
    class SystemTestZone : public GameZone
    {
    public:
    
        SystemTestZone(const ZoneCreationContext& context);
        ~SystemTestZone();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    protected:

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const game::Config m_game_config;
    };
}
