
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include <cstdint>

namespace game
{
    class CirculatingBehaviour
    {
    public:

        CirculatingBehaviour(mono::TransformSystem* transform_system);
        void Initialize(uint32_t position_entity_id, float radius, mono::IBody* body);
        void Run(const mono::UpdateContext& update_context);

    private:

        mono::TransformSystem* m_transform_system;
        uint32_t m_position_entity_id;
        mono::IBody* m_body;

        float m_forward_velocity;
        float m_angular_velocity; // Degrees per second
        float m_radius;
        float m_current_angle_rad;
    };
}
