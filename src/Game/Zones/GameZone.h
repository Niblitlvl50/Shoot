
#pragma once

#include "Zone/ZoneBase.h"
#include "ZoneCreationContext.h"
#include "WorldFile.h"

#include <memory>

class ImGuiInputHandler;

namespace game
{
    class GameZone : public mono::ZoneBase
    {
    public:

        GameZone(const ZoneCreationContext& context);
        ~GameZone();
        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    protected:

        std::unique_ptr<class IGameMode> CreateGameMode();

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const char* m_world_file;

        LevelData m_leveldata;
        std::unique_ptr<ImGuiInputHandler> m_debug_input;

        class HealthbarDrawer* m_healthbar_drawer;
        class RegionDrawer* m_region_ui;
        class MissionSystemDrawer* m_mission_ui;
        class DebugUpdater* m_debug_updater;
        std::unique_ptr<IGameMode> m_game_mode;
    };
}
