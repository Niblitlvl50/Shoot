
#pragma once

#include "ZoneCreationContext.h"
//#include "Zone/ZoneBase.h"
#include "Zone/PhysicsZone.h"
#include "GameConfig.h"
#include "EventHandler/EventToken.h"


namespace game
{
    struct TextMessage;
    struct PositionalMessage;
    struct SpawnMessage;
    struct DespawnMessage;
    struct AnimationMessage;

    class RemoteZone : public mono::PhysicsZone
    {
    public:
    
        RemoteZone(const ZoneCreationContext& context);
        ~RemoteZone();

        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

        bool HandleText(const TextMessage& text_message);
        bool HandlePosMessage(const PositionalMessage& pos_message);
        bool HandleSpawnMessage(const SpawnMessage& spawn_message);
        bool HandleDespawnMessage(const DespawnMessage& despawn_message);
        bool HandleAnimMessage(const AnimationMessage& anim_message);

    private:

        void Accept(mono::IRenderer& renderer) override;

        mono::EventHandler& m_event_handler;
        const game::Config m_game_config;

        std::shared_ptr<class MessageDispatcher> m_dispatcher;
        std::unique_ptr<class RemoteConnection> m_connection;

        mono::EventToken<game::TextMessage> m_text_token;
        mono::EventToken<game::PositionalMessage> m_pos_token;
        mono::EventToken<game::SpawnMessage> m_spawn_token;
        mono::EventToken<game::DespawnMessage> m_despawn_token;
        mono::EventToken<game::AnimationMessage> m_anim_token;

        std::shared_ptr<class ConsoleDrawer> m_console_drawer;
    };
}
