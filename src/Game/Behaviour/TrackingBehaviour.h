
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

        void Init(mono::EventHandler& event_handler, Enemy* enemy);
        void Exit();

        TrackingResult Run(unsigned int delta);

    private:

        bool UpdatePath();

        mono::EventHandler* m_event_handler;
        Enemy* m_enemy;
        
        mono::IBodyPtr m_control_body;
        mono::IConstraintPtr m_spring;
        mono::IPathPtr m_path;
        std::shared_ptr<class AStarPathDrawer> m_astar_drawer;

        unsigned int m_tracking_timer;
        float m_current_position;
    };
}
