
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include <cstdint>

namespace game
{
    extern bool g_draw_client_viewport;
    extern bool g_draw_navmesh;
    extern bool g_draw_transformsystem;
    extern bool g_draw_physics;
    extern bool g_draw_audio;
    extern bool g_draw_triggers;
    extern uint32_t g_draw_physics_subcomponents;
    extern bool g_draw_fps;
    extern bool g_draw_physics_stats;
    extern bool g_draw_particle_stats;
    extern bool g_draw_network_stats;
    extern bool g_draw_position_prediction;

    class DebugUpdater : public mono::IDrawable
    {
    public:
        DebugUpdater(mono::EventHandler* event_handler);
        ~DebugUpdater();
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:
        mono::EventHandler* m_event_handler;
        mutable bool m_draw_debug_menu;
        mutable bool m_draw_debug_players;
        mono::EventToken<event::KeyUpEvent> m_keyup_token;
    };
}
