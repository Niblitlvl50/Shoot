
#pragma once

#include "Enemy.h"
#include "StateMachine.h"

#include "MonoPtrFwd.h"

namespace game
{
    enum class InvaderStates
    {
        IDLE,
        TRACKING,
        ATTACKING
    };

    class InvaderController : public IEnemyController
    {
    public:

        InvaderController(mono::EventHandler& event_handler);
        virtual ~InvaderController();

        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        void ToIdle();
        void ToTracking();
        void ToAttacking();
        void Idle(unsigned int delta);
        void Tracking(unsigned int delta);
        void Attacking(unsigned int delta);

        bool UpdatePath();
        
        mono::EventHandler& m_event_handler;

        Enemy* m_enemy;
        mono::IBodyPtr m_control_body;
        mono::IConstraintPtr m_spring;
        std::unique_ptr<class IWeaponSystem> m_weapon;

        std::shared_ptr<class AStarPathDrawer> m_astar_drawer;

        mono::IPathPtr m_path;

        unsigned int m_idle_timer;
        unsigned int m_tracking_timer;
        float m_current_position;

        using InvaderStateMachine = StateMachine<InvaderStates, unsigned int>;
        InvaderStateMachine m_states;
    };
}
