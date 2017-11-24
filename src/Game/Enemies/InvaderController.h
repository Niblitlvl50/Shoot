
#pragma once

#include "Enemy.h"
#include "StateMachine.h"

#include "MonoPtrFwd.h"

namespace game
{
    enum class InvaderStates
    {
        IDLE,
        TRACKING
    };

    class InvaderController : public IEnemyController
    {
    public:

        InvaderController(mono::EventHandler& event_handler);
        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        void ToIdle();
        void ToTracking();
        void Idle(unsigned int delta);
        void Tracking(unsigned int delta);
        
        Enemy* m_enemy;
        mono::IPathPtr m_path;

        unsigned int m_idle_timer;
        float m_current_position;

        using InvaderStateMachine = StateMachine<InvaderStates, unsigned int>;
        InvaderStateMachine m_states;
    };
}
