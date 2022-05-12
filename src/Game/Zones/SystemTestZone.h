
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
    
        SystemTestZone(const ZoneCreationContext& context, const char* zone_file);
        ~SystemTestZone();

        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    protected:

        std::unique_ptr<IGameMode> CreateGameMode() override;

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const game::Config m_game_config;

        std::unique_ptr<class RegionDrawer> m_region_ui;
        std::unique_ptr<class FogOverlay> m_fog;
    };

    class WorldZone : public SystemTestZone
    {
    public:
        WorldZone(const ZoneCreationContext& context)
            : SystemTestZone(context, "res/worlds/world.components")
        { }
    };

    class TinyArenaZone : public SystemTestZone
    {
    public:
        TinyArenaZone(const ZoneCreationContext& context)
            : SystemTestZone(context, "res/worlds/tiny_arena.components")
        { }
    };

    class BossArenaZone : public SystemTestZone
    {
    public:

        BossArenaZone(const ZoneCreationContext& context)
            : SystemTestZone(context, "res/worlds/boss_arena.components")
        { }
    };

    class EnemyTestbedZone : public SystemTestZone
    {
    public:
        EnemyTestbedZone(const ZoneCreationContext& context)
            : SystemTestZone(context, "res/worlds/enemy_testbed.components")
        { }
    };

    class TutorialDeliveryZone : public SystemTestZone
    {
    public:
        TutorialDeliveryZone(const ZoneCreationContext& context)
            : SystemTestZone(context, "res/worlds/tutorial_delivery.components")
        { }
    };
}
