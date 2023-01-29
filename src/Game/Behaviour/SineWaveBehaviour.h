
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
        SineWaveBehaviour(mono::IBody* body);

        void SetBody(mono::IBody* body);
        void SetHeading(float heading);
        void Run(const mono::UpdateContext& update_context);

    private:

        mono::IBody* m_body;

        float m_radians;
        float m_sine_speed_deg_s;
        float m_magnitude;
    };
}
