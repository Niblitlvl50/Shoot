
#pragma once

#include "MonoPtrFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Math/Vector.h"
#include <cstdint>

namespace game
{
    enum class TrackingState
    {
        AT_TARGET,
        TRACKING,
        NO_PATH
    };

    inline const char* TrackingResultToString(TrackingState result)
    {
        switch(result)
        {
        case TrackingState::AT_TARGET:
            return "At Target";
        case TrackingState::TRACKING:
            return "Tracking";
        case TrackingState::NO_PATH:
            return "No Path";
        };
    }

    struct TrackingResult
    {
        TrackingState state;
        float distance_to_target;
    };

    class TrackingBehaviour
    {
    public:

        TrackingBehaviour();
        ~TrackingBehaviour();

        void Init(mono::IBody* body, mono::PhysicsSystem* physics_system, class NavigationSystem* navigation_system);
        void Release();

        void SetTrackingSpeed(float meter_per_second);
        bool UpdatePath(const math::Vector& tracking_position);
        void UpdateEntityPosition();
        const math::Vector& GetTrackingPosition() const;

        TrackingResult Run(const mono::UpdateContext& update_context, const math::Vector& tracking_position);

    private:

        mono::IBody* m_entity_body;
        mono::PhysicsSystem* m_physics_system;
        game::NavigationSystem* m_navigation_system;

        math::Vector m_tracking_position;
        float m_current_position;
        float m_meter_per_second;

        float m_time_since_last_update;

        mono::IBody* m_control_body;
        mono::IConstraint* m_spring;
        mono::IPathPtr m_path;
    };
}
