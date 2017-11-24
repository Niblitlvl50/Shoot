
#pragma once

#include "Zone/PhysicsZone.h"
#include "EventHandler/EventToken.h"

#include "Spawner.h"
#include "DamageController.h"
#include "Player/PlayerDaemon.h"
#include "Network/RemoteConnection.h"
#include "Hud/Healthbar.h"

#include "Navigation/NavMesh.h"

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
        
        TestZone(mono::EventHandler& eventHandler);
        ~TestZone();
                
        void OnLoad(mono::ICameraPtr& camera) override;
        void OnUnload() override;
        
        bool SpawnEntity(const game::SpawnEntityEvent& event);
        bool SpawnPhysicsEntity(const game::SpawnPhysicsEntityEvent& event);
        bool OnRemoveEntity(const game::RemoveEntityEvent& event);
        bool OnShockwaveEvent(const game::ShockwaveEvent& event);
        bool OnDamageEvent(const game::DamageEvent& event);
        bool OnSpawnConstraint(const game::SpawnConstraintEvent& event);
        bool OnDespawnConstraint(const game::DespawnConstraintEvent& event);

    private:

        void AddEntity(const mono::IEntityPtr& entity, int layer, DestroyedFunction destroyed_func);
        void AddPhysicsEntity(const mono::IPhysicsEntityPtr& entity, int layer, DestroyedFunction destroyed_func);

        void RemovePhysicsEntity(const mono::IPhysicsEntityPtr& entity) override;
        void RemoveEntity(const mono::IEntityPtr& entity) override;

        mono::EventToken<game::SpawnEntityEvent> mSpawnEntityToken;
        mono::EventToken<game::SpawnPhysicsEntityEvent> mSpawnPhysicsEntityToken;
        mono::EventToken<game::RemoveEntityEvent> mRemoveEntityByIdToken;
        mono::EventToken<game::ShockwaveEvent> mShockwaveEventToken;
        mono::EventToken<game::DamageEvent> mDamageEventToken;
        mono::EventToken<game::SpawnConstraintEvent> m_spawnConstraintToken;
        mono::EventToken<game::DespawnConstraintEvent> m_despawnConstraintToken;
        
        mono::EventHandler& mEventHandler;
        std::unique_ptr<Spawner> m_spawner;
        std::unique_ptr<PlayerDaemon> m_player_daemon;
        DamageController m_damageController;
        mono::ISoundPtr m_backgroundMusic;
        RemoteConnection m_connection;

        std::vector<Healthbar> m_healthbars;

        NavmeshContext m_navmesh;
    };
}
