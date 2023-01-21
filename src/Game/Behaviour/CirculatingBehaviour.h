
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    class CirculatingBehaviour
    {
    public:

        CirculatingBehaviour();
        CirculatingBehaviour(mono::IBody* body);

        void SetBody(mono::IBody* body);
        void SetForwardVelocity(float velocity);
        void SetAngularVelocity(float degrees_per_second);

        void Run(const mono::UpdateContext& update_context);

    private:

        mono::IBody* m_body;

        float m_forward_velocity;
        float m_angular_velocity; // Degrees per second
        float m_current_angle_rad;
    };
}
