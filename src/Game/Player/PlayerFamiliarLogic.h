
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"

#include "Math/Vector.h"

namespace game
{
    class CameraSystem;

    class PlayerFamiliarLogic : public IEntityLogic
    {
    public:

        PlayerFamiliarLogic(
            uint32_t entity_id,
            uint32_t owner_entity_id,
            mono::SystemContext* system_context);
        ~PlayerFamiliarLogic();
        void Update(const mono::UpdateContext& update_context) override;

    private:

        const uint32_t m_entity_id;
        const uint32_t m_owner_entity_id;

        mono::SpriteSystem* m_sprite_system;
        mono::TransformSystem* m_transform_system;
        mono::ParticleSystem* m_particle_system;
        mono::LightSystem* m_light_system;
        game::CameraSystem* m_camera_system;

        bool m_last_show_state;
        float m_idle_timer;
        math::Vector m_target_screen_position;
        math::Vector m_move_velocity;
    };
}
