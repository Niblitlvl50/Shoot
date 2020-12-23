
#pragma once

#include "GameZone.h"
#include "ZoneCreationContext.h"
#include "GameConfig.h"

#include <memory>

class ImGuiInputHandler;

namespace game
{
    class SystemTestZone : public GameZone
    {
    public:
    
        SystemTestZone(const ZoneCreationContext& context);
        ~SystemTestZone();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    private:

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const game::Config m_game_config;

        uint32_t m_level_completed_trigger;
        int m_next_zone;

        std::unique_ptr<class PlayerDaemon> m_player_daemon;
        std::unique_ptr<class ServerManager> m_server_manager;
    };
}
