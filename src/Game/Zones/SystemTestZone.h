
#pragma once

#include "GameZone.h"
#include "ZoneCreationContext.h"
#include "GameConfig.h"
#include "EventHandler/EventToken.h"

#include <memory>

class ImGuiInputHandler;

namespace game
{
    class SystemTestZone : public GameZone
    {
    public:
    
        SystemTestZone(const ZoneCreationContext& context, const char* zone_file);
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
        std::unique_ptr<class PlayerDeathScreen> m_player_death_screen;
        std::unique_ptr<class PlayerUIElement> m_player_ui;
        std::unique_ptr<class FogOverlay> m_fog;

        std::unique_ptr<class AngelDust> m_angeldust_effect;

        mono::EventToken<struct GameOverEvent> m_gameover_token;
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
}
