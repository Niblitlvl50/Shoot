
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"

#include "Events/EventFwd.h"
#include "EventHandler/EventToken.h"

#include "Math/Vector.h"

namespace game
{
    class PlayerFamiliarLogic : public IEntityLogic
    {
    public:

        PlayerFamiliarLogic(
            uint32_t entity_id,
            mono::EventHandler* event_handler,
            mono::SystemContext* system_context);
        ~PlayerFamiliarLogic();
        void Update(const mono::UpdateContext& update_context) override;

    private:

        mono::EventResult OnMouseMotion(const event::MouseMotionEvent& event);

        const uint32_t m_entity_id;
        mono::EventHandler* m_event_handler;
        mono::SpriteSystem* m_sprite_system;
        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::LightSystem* m_light_system;

        mono::EventToken<event::MouseMotionEvent> m_mouse_motion_token;

        bool m_last_show_state;
        float m_idle_timer;
        math::Vector m_target_position;
        math::Vector m_move_velocity;
    };
}
