
#pragma once

#include "Zone/ZoneBase.h"
#include "ZoneCreationContext.h"
#include "Navigation/NavmeshData.h"

#include <memory>

class ImGuiInputHandler;

namespace game
{
    class GameZone : public mono::ZoneBase
    {
    public:

        GameZone(const ZoneCreationContext& context, const char* world_file);
        ~GameZone();
        void OnLoad(mono::ICamera* camera) override;
        int OnUnload() override;

    protected:

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;
        const char* m_world_file;

        math::Vector m_player_spawn_point;

        std::unique_ptr<ImGuiInputHandler> m_debug_input;
        NavmeshContext m_navmesh;
        std::vector<uint32_t> m_loaded_entities;
    };
}
