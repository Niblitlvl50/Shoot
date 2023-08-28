
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "Physics/PhysicsFwd.h"

#include <array>
#include <cstdint>

namespace game
{
    struct HomingResult
    {
        float new_heading;
        float distance_to_target;
        bool is_stuck;
    };

    class HomingBehaviour
    {
    public:

        HomingBehaviour();
        void SetBody(mono::IBody* body);
        
        void SetHeading(float heading);
        void SetTargetPosition(const math::Vector& position);
        const math::Vector& GetTargetPosition() const;

        void SetForwardVelocity(float velocity);
        void SetAngularVelocity(float degrees_per_second);
        void SetHomingStartDelay(float delay_s);
        void SetHomingDuration(float duration_s);

        HomingResult Run(const mono::UpdateContext& update_context);

    private:

        bool CheckIfBlocked(std::array<math::Vector, 10>& position_samples, uint32_t current_timestamp, const math::Vector& current_position);

        mono::IBody* m_body;
        float m_current_heading;

        math::Vector m_target_position;
        float m_forward_velocity;
        float m_angular_velocity; // Degrees per second

        float m_homing_start_delay_s;
        float m_homing_duration_s;

        int m_sample_index;
        uint32_t m_timestamp;
        std::array<math::Vector, 10> m_position_samples;
    };
}
