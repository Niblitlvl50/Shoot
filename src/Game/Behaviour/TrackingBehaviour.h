
#pragma once

#include "Physics/CMFwd.h"
#include "MonoPtrFwd.h"

namespace game
{
    class Enemy;

    enum class TrackingResult
    {
        AT_TARGET,
        TRACKING,
        NO_PATH
    };

    class TrackingBehaviour
    {
    public:

        TrackingBehaviour(Enemy* enemy, mono::EventHandler& event_handler);
        ~TrackingBehaviour();

        void SetTrackingSpeed(float meter_per_second);
        TrackingResult Run(unsigned int delta);

    private:

        bool UpdatePath();

        Enemy* m_enemy;
        mono::EventHandler& m_event_handler;
        unsigned int m_tracking_timer;
        float m_current_position;
        float m_meter_per_second;
        
        mono::IBodyPtr m_control_body;
        mono::IConstraintPtr m_spring;
        mono::IPathPtr m_path;
        std::shared_ptr<class AStarPathDrawer> m_astar_drawer;
    };
}
