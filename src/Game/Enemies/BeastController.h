
#pragma once

#include "Enemy.h"
#include "StateMachine.h"

namespace game
{
    class BeastController : public IEnemyController
    {
    public:

        enum class BeastStates
        {
            IDLE,
            TRACKING,
            ATTACKING
        };

        BeastController(mono::EventHandler& event_handler);
        void Initialize(class Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:
    
        void ToIdle();
        void ToTracking();
        void ToAttacking();
        void Idle(unsigned int delta);
        void Tracking(unsigned int delta);
        void Attacking(unsigned int delta);

        class Enemy* m_enemy;

        using BeastStateMachine = StateMachine<BeastStates, unsigned int>;
        BeastStateMachine m_states;

        unsigned int m_timer;
    };
}
