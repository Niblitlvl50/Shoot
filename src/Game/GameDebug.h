
#pragma once

#include "IUpdatable.h"
#include "MonoFwd.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include <cstdint>

namespace game
{
    extern bool g_draw_client_viewport;
    extern bool g_draw_navmesh;
    extern bool g_draw_transformsystem;
    extern bool g_draw_physics;
    extern uint32_t g_draw_physics_subcomponents;
    extern bool g_draw_fps;
    extern bool g_draw_physics_stats;
    extern bool g_draw_particle_stats;
    extern bool g_draw_network_stats;

    class DebugUpdater : public mono::IUpdatable
    {
    public:
        DebugUpdater(mono::EventHandler* event_handler);
        ~DebugUpdater();
        void doUpdate(const mono::UpdateContext& update_context) override;

    private:
        mono::EventHandler* m_event_handler;
        bool m_draw;
        mono::EventToken<event::KeyUpEvent> m_keyup_token;
    };
}
