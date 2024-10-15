
#pragma once

#include "MonoFwd.h"
#include "Rendering/IDrawable.h"
#include "IUpdatable.h"
#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"
#include "Util/FpsCounter.h"
#include "Util/CircularVector.h"

#include "GameDebugVariables.h"

#include <cstdint>
#include <memory>

namespace game
{
    class DamageSystem;
    class EntityLogicSystem;

    class DebugUpdater : public mono::IUpdatable, public mono::IDrawable
    {
    public:
        DebugUpdater(
            mono::SystemContext* system_context,
            mono::EventHandler* event_handler,
            mono::IRenderer* renderer);
        ~DebugUpdater();
        void Update(const mono::UpdateContext& update_context) override;
        void Draw(mono::IRenderer& renderer) const override;
        math::Quad BoundingBox() const override;

    private:

        mono::EventHandler* m_event_handler;
        mutable bool m_draw_debug_menu;
        mutable bool m_draw_trigger_input;
        bool m_pause;

        mono::TriggerSystem* m_trigger_system;
        mono::IEntityManager* m_entity_manager;
        DamageSystem* m_damage_system;
        EntityLogicSystem* m_logic_system;

        mono::EventToken<event::KeyUpEvent> m_keyup_token;

        mono::FpsCounter m_counter;
        mono::CircularVector<float, 1000> m_frame_times;

        class PlayerDebugHandler;
        std::unique_ptr<PlayerDebugHandler> m_player_debug_handler;
    };
}
