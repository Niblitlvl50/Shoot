
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    struct HomingResult
    {
        float new_heading;
        float distance_to_target;
    };

    class HomingBehaviour
    {
    public:

        HomingBehaviour();
        HomingBehaviour(mono::IBody* body);

        void SetBody(mono::IBody* body);
        
        void SetHeading(float heading);
        void SetTargetPosition(const math::Vector& position);

        void SetForwardVelocity(float velocity);
        void SetAngularVelocity(float degrees_per_second);

        HomingResult Run(const mono::UpdateContext& update_context);

    private:

        mono::IBody* m_body;
        float m_current_heading;

        math::Vector m_target_position;
        float m_forward_velocity;
        float m_angular_velocity; // Degrees per second
    };
}
