
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/PhysicsZone.h"
#include "GameConfig.h"
#include "EventHandler/EventToken.h"

namespace network
{
    struct Address;
}

namespace game
{
    struct TextMessage;
    struct SpawnMessage;
    struct SpriteMessage;
    struct PingMessage;

    class RemoteZone : public mono::PhysicsZone
    {
    public:
    
        RemoteZone(const ZoneCreationContext& context);
        ~RemoteZone();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

        bool HandleText(const TextMessage& text_message);
        bool HandleSpawnMessage(const SpawnMessage& spawn_message);
        bool HandleSpriteMessage(const SpriteMessage& sprite_message);
        bool HandlePingMessage(const PingMessage& ping_message);

    private:

        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;
        const game::Config m_game_config;

        std::shared_ptr<class ClientManager> m_client_manager;

        mono::EventToken<game::TextMessage> m_text_token;
        mono::EventToken<game::SpawnMessage> m_spawn_token;
        mono::EventToken<game::SpriteMessage> m_sprite_token;
        mono::EventToken<game::PingMessage> m_ping_token;

        std::shared_ptr<class ConsoleDrawer> m_console_drawer;
        std::unique_ptr<class ClientPlayerDaemon> m_player_daemon;
    };
}
