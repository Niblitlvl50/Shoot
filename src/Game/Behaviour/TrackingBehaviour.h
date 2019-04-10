
#pragma once

#include "Physics/PhysicsFwd.h"
#include "MonoPtrFwd.h"

namespace game
{
    enum class TrackingResult
    {
        AT_TARGET,
        TRACKING,
        NO_PATH
    };

    class TrackingBehaviour
    {
    public:

        TrackingBehaviour(mono::IBody* body, mono::PhysicsSystem* physics_system);
        ~TrackingBehaviour();

        void SetTrackingSpeed(float meter_per_second);
        TrackingResult Run(uint32_t delta_ms);

    private:

        bool UpdatePath();

        mono::IBody* m_entity_body;
        mono::PhysicsSystem* m_physics_system;
        uint32_t m_tracking_timer;
        float m_current_position;
        float m_meter_per_second;
        
        mono::IBody* m_control_body;
        mono::IConstraint* m_spring;
        mono::IPathPtr m_path;
        std::shared_ptr<class AStarPathDrawer> m_astar_drawer;
    };
}
