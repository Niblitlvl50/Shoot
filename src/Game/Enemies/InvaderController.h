
#pragma once

#include "Enemy.h"
#include "StateMachine.h"

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
        
        mono::EventHandler& m_event_handler;

        Enemy* m_enemy;
        std::unique_ptr<class IWeaponSystem> m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;

        unsigned int m_idle_timer;
        using InvaderStateMachine = StateMachine<InvaderStates, unsigned int>;
        InvaderStateMachine m_states;
    };
}
