
#pragma once

#include "ZoneCreationContext.h"
#include "Zone/ZoneBase.h"
#include "EventHandler/EventToken.h"
#include "Navigation/NavMesh.h"

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

        bool SpawnConstraint(const game::SpawnConstraintEvent& event);
        bool DespawnConstraint(const game::DespawnConstraintEvent& event);

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        std::vector<uint32_t> m_loaded_entities;
        std::unique_ptr<class PlayerDaemon> m_player_daemon;
        std::shared_ptr<class ConsoleDrawer> m_console_drawer;
        std::vector<struct Pickup> m_pickups;
        
        NavmeshContext m_navmesh;

        mono::EventToken<game::SpawnConstraintEvent> m_spawn_constraint_token;
        mono::EventToken<game::DespawnConstraintEvent> m_despawn_constraint_token;
    };
}
