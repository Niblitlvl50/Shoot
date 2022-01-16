
#pragma once

#include "Zone/ZoneBase.h"
#include "ZoneCreationContext.h"
#include "Navigation/NavmeshData.h"
#include "WorldFile.h"

#include <memory>

class ImGuiInputHandler;

namespace game
{
    class GameZone : public mono::ZoneBase
    {
    public:

        GameZone(const ZoneCreationContext& context, const char* world_file);
        ~GameZone();
        void OnLoad(mono::ICamera* camera, mono::IRenderer* renderer) override;
        int OnUnload() override;

    protected:

        virtual std::unique_ptr<class IGameMode> CreateGameMode();

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const char* m_world_file;

        LevelData m_leveldata;

        std::unique_ptr<ImGuiInputHandler> m_debug_input;
        NavmeshContext m_navmesh;

        std::unique_ptr<IGameMode> m_game_mode;
    };
}
