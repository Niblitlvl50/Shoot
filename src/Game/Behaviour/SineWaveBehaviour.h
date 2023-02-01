
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"

namespace game
{
    class SineWaveBehaviour
    {
    public:

        SineWaveBehaviour();
        void Initialize(mono::IBody* body, const math::Vector& position);
        void Run(const mono::UpdateContext& update_context);

    private:

        mono::IBody* m_body;
        math::Vector m_position;
        math::Vector m_velocity;

        float m_radians;
        float m_sine_speed_deg_s;
        float m_magnitude;
    };
}
