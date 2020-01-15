
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "GameConfig.h"
#include "EventHandler/EventToken.h"
#include "GameMode/CaptureTheFlagScore.h"

namespace network
{
    struct Address;
}

namespace game
{
    struct TextMessage;
    struct SpawnMessage;
    struct SpriteMessage;

    class RemoteZone : public mono::ZoneBase
    {
    public:
    
        RemoteZone(const ZoneCreationContext& context);
        ~RemoteZone();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

        bool HandleText(const TextMessage& text_message);
        bool HandleSpawnMessage(const SpawnMessage& spawn_message);
        bool HandleSpriteMessage(const SpriteMessage& sprite_message);

    private:

        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;
        const game::Config m_game_config;

        std::shared_ptr<class ClientManager> m_client_manager;

        mono::EventToken<game::TextMessage> m_text_token;
        mono::EventToken<game::SpawnMessage> m_spawn_token;
        mono::EventToken<game::SpriteMessage> m_sprite_token;

        std::shared_ptr<class ConsoleDrawer> m_console_drawer;
        std::unique_ptr<class ClientPlayerDaemon> m_player_daemon;

        CaptureTheFlagScore m_ctf_score;
    };
}
