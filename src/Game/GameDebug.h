
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "Util/FpsCounter.h"

#include <cstdint>
#include <memory>

namespace game
{
    extern bool g_draw_client_viewport;
    extern bool g_draw_navmesh;
    extern bool g_draw_transformsystem;
    extern bool g_draw_triggers;
    extern bool g_draw_physics;
    extern bool g_interact_physics;
    extern uint32_t g_draw_physics_subcomponents;
    extern bool g_draw_physics_stats;
    extern bool g_draw_particle_stats;
    extern bool g_draw_network_stats;
    extern bool g_draw_position_prediction;
    extern bool g_draw_debug_players;
    extern bool g_draw_spawn_points;

    class TriggerSystem;
    class DamageSystem;

    class DebugUpdater : public mono::IDrawable
    {
    public:
        DebugUpdater(
            TriggerSystem* trigger_system,
            DamageSystem* damage_system,
            mono::TransformSystem* transform_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler);
        ~DebugUpdater();
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        TriggerSystem* m_trigger_system;
        DamageSystem* m_damage_system;
        mono::EventHandler* m_event_handler;
        mono::EventToken<event::KeyUpEvent> m_keyup_token;

        mutable bool m_draw_debug_menu;
        mutable bool m_draw_trigger_input;
        mutable mono::FpsCounter m_counter;

        class PlayerDebugHandler;
        std::unique_ptr<PlayerDebugHandler> m_player_debug_handler;
    };
}
