
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/PhysicsFwd.h"

namespace game
{
    class HomingBehaviour
    {
    public:

        HomingBehaviour(mono::IBody* body, mono::PhysicsSystem* physics_system);
        ~HomingBehaviour();

        void SetHomingPosition(const math::Vector& position);

    private:

        mono::PhysicsSystem* m_physics_system;
        mono::IBody* m_control_body;
        mono::IConstraint* m_spring;
    };
}
