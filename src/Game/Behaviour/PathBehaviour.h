
#pragma once

#include "MonoFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"

#include <cstdint>

namespace game
{
    class PathBehaviour
    {
    public:

        PathBehaviour(mono::IBody* body, const mono::IPath* path, mono::PhysicsSystem* physics_system);
        ~PathBehaviour();

        void SetTrackingSpeed(float meter_per_second);
        void Run(uint32_t delta_ms);

    private:

        const mono::IPath* m_path;
        mono::PhysicsSystem* m_physics_system;

        float m_current_position;
        float m_meter_per_second;

        mono::IBody* m_control_body;
        mono::IConstraint* m_spring;
        math::Vector m_point;
    };
}
