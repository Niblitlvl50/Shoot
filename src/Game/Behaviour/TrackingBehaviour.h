
#pragma once

#include "MonoPtrFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    enum class TrackingResult
    {
        AT_TARGET,
        TRACKING,
        NO_PATH
    };

    inline const char* TrackingResultToString(TrackingResult result)
    {
        switch(result)
        {
        case TrackingResult::AT_TARGET:
            return "At Target";
        case TrackingResult::TRACKING:
            return "Tracking";
        case TrackingResult::NO_PATH:
            return "No Path";
        };
    }

    class TrackingBehaviour
    {
    public:

        TrackingBehaviour(mono::IBody* body, mono::PhysicsSystem* physics_system);
        ~TrackingBehaviour();

        void SetTrackingSpeed(float meter_per_second);
        bool UpdatePath(const math::Vector& tracking_position);

        TrackingResult Run(const mono::UpdateContext& update_context, const math::Vector& tracking_position);

    private:

        mono::IBody* m_entity_body;
        mono::PhysicsSystem* m_physics_system;

        math::Vector m_tracking_position;
        float m_current_position;
        float m_meter_per_second;

        mono::IBody* m_control_body;
        mono::IConstraint* m_spring;
        mono::IPathPtr m_path;
    };
}
