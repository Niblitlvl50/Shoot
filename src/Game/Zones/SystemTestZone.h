
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "EventHandler/EventToken.h"
#include "Navigation/NavMesh.h"

#include "GameConfig.h"

class ImGuiInputHandler;

namespace game
{
    struct SpawnConstraintEvent;
    struct DespawnConstraintEvent;

    class SystemTestZone : public mono::ZoneBase
    {
    public:
    
        SystemTestZone(const ZoneCreationContext& context);
        ~SystemTestZone();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

    private:

        mono::EventResult HandleText(const struct TextMessage& text_message);
        mono::EventResult HandleRemoteCamera(const struct RemoteCameraMessage& message);

        std::unique_ptr<ImGuiInputHandler> m_debug_input;
        mono::ICameraPtr m_camera;

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const game::Config m_game_config;

        std::vector<uint32_t> m_loaded_entities;
        std::unique_ptr<class PlayerDaemon> m_player_daemon;
        std::shared_ptr<class ConsoleDrawer> m_console_drawer;
        std::vector<struct Pickup> m_pickups;
        
        std::shared_ptr<class ServerManager> m_server_manager;

        NavmeshContext m_navmesh;

        mono::EventToken<game::TextMessage> m_text_func_token;
        mono::EventToken<game::RemoteCameraMessage> m_camera_func_token;
    };
}
