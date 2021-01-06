
#pragma once

#include "MonoFwd.h"
#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "GameConfig.h"
#include "EventHandler/EventToken.h"

#include <memory>

class ImGuiInputHandler;

namespace network
{
    struct Address;
}

namespace game
{
    struct LevelMetadataMessage;
    struct TextMessage;
    struct SpawnMessage;
    struct SpriteMessage;
    struct TransformMessage;
    struct DamageInfoMessage;

    class DamageSystem;

    class RemoteZone : public mono::ZoneBase
    {
    public:
    
        RemoteZone(const ZoneCreationContext& context);
        ~RemoteZone();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

        mono::EventResult HandleLevelMetadata(const LevelMetadataMessage& metadata_message);
        mono::EventResult HandleText(const TextMessage& text_message);
        mono::EventResult HandleSpawnMessage(const SpawnMessage& spawn_message);
        mono::EventResult HandleSpriteMessage(const SpriteMessage& sprite_message);
        mono::EventResult HandleTransformMessage(const TransformMessage& transform_message);
        mono::EventResult HandleDamageInfoMessage(const DamageInfoMessage& damageinfo_message);

    private:

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const game::Config m_game_config;

        mono::ICamera* m_camera;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;
        game::DamageSystem* m_damage_system;
        class PositionPredictionSystem* m_position_prediction_system;
        class SpawnPredictionSystem* m_spawn_prediction_system;

        mono::EventToken<game::LevelMetadataMessage> m_metadata_token;
        mono::EventToken<game::TextMessage> m_text_token;
        mono::EventToken<game::SpawnMessage> m_spawn_token;
        mono::EventToken<game::SpriteMessage> m_sprite_token;
        mono::EventToken<game::TransformMessage> m_transform_token;
        mono::EventToken<game::DamageInfoMessage> m_damageinfo_token;

        std::unique_ptr<class ConsoleDrawer> m_console_drawer;
        std::unique_ptr<class ClientPlayerDaemon> m_player_daemon;
        std::unique_ptr<ImGuiInputHandler> m_debug_input;
    };
}
