
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/PhysicsZone.h"
#include "EventHandler/EventToken.h"

#include "Spawner.h"
#include "Player/PlayerDaemon.h"
#include "Hud/Healthbar.h"
#include "Pickups/Ammo.h"

#include "Navigation/NavMesh.h"

#include "Network/RemoteConnection.h"
#include "Network/MessageDispatcher.h"
#include "Network/NetworkBeacon.h"

#include "GameConfig.h"

#include <vector>
#include <memory>

namespace game
{
    struct SpawnEntityEvent;
    struct SpawnPhysicsEntityEvent;
    struct RemoveEntityEvent;
    struct ShockwaveEvent;
    struct DamageEvent;
    struct SpawnConstraintEvent;
    struct DespawnConstraintEvent;

    class TestZone : public mono::PhysicsZone
    {
    public:
        
        TestZone(const ZoneCreationContext& context);
        ~TestZone();
                
        void OnLoad(mono::ICameraPtr& camera) override;
        int OnUnload() override;

        bool SpawnEntity(const game::SpawnEntityEvent& event);
        bool SpawnPhysicsEntity(const game::SpawnPhysicsEntityEvent& event);
        bool OnRemoveEntity(const game::RemoveEntityEvent& event);
        bool OnShockwaveEvent(const game::ShockwaveEvent& event);
        bool OnDamageEvent(const game::DamageEvent& event);
        bool OnSpawnConstraint(const game::SpawnConstraintEvent& event);
        bool OnDespawnConstraint(const game::DespawnConstraintEvent& event);

        bool HandleText(const TextMessage& text_message);
        bool HandlePosMessage(const PositionalMessage& pos_message);
        bool HandleSpawnMessage(const SpawnMessage& spawn_message);

    private:

        void RemovePhysicsEntity(const mono::IPhysicsEntityPtr& entity) override;
        void RemoveEntity(const mono::IEntityPtr& entity) override;

        void Accept(mono::IRenderer& renderer) override;

        mono::EventToken<game::SpawnEntityEvent> m_spawn_entity_token;
        mono::EventToken<game::SpawnPhysicsEntityEvent> m_spawn_physics_entity_token;
        mono::EventToken<game::RemoveEntityEvent> m_remove_entity_by_id_token;
        mono::EventToken<game::ShockwaveEvent> m_shockwave_event_token;
        mono::EventToken<game::DamageEvent> m_damage_event_token;
        mono::EventToken<game::SpawnConstraintEvent> m_spawnConstraintToken;
        mono::EventToken<game::DespawnConstraintEvent> m_despawnConstraintToken;

        mono::EventToken<game::TextMessage> m_text_func_token;
        mono::EventToken<game::PositionalMessage> m_pos_func_token;
        mono::EventToken<game::SpawnMessage> m_spawn_func_token;

        const game::Config m_game_config;
        mono::EventHandler& m_event_handler;
        mono::SystemContext* m_system_context;
        
        std::unique_ptr<Spawner> m_enemy_spawner;
        std::unique_ptr<PlayerDaemon> m_player_daemon;
        mono::ISoundPtr m_background_music;

        NetworkBeacon m_beacon;
        std::shared_ptr<MessageDispatcher> m_dispatcher;
        std::unique_ptr<RemoteConnection> m_remote_connection;

        //std::vector<Healthbar> m_healthbars;

        NavmeshContext m_navmesh;
        std::shared_ptr<class GibSystem> m_gib_system;
        std::shared_ptr<class ConsoleDrawer> m_console_drawer;

        std::vector<Pickup> m_pickups;
    };
}
