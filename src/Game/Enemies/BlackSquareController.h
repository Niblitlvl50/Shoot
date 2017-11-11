
#pragma once

#include "MonoPtrFwd.h"
#include "Enemy.h"
#include "StateMachine.h"

namespace game
{
    class BlackSquareController : public IEnemyController
    {
    public:

        BlackSquareController(float trigger_distance, mono::EventHandler& event_handler);
        virtual ~BlackSquareController();

        virtual void Initialize(Enemy* enemy);
        virtual void doUpdate(unsigned int delta);

    private:

        enum class States
        {
            SLEEPING,
            AWAKE,
            HUNT
        };

        void ToSleep();
        void ToAwake();
        void ToHunt();

        void SleepState(unsigned int delta);
        void AwakeState(unsigned int delta);
        void HuntState(unsigned int delta);

        const float m_triggerDistance;
        mono::EventHandler& m_eventHandler;

        using MyStateMachine = StateMachine<States, unsigned int>;
        MyStateMachine m_states;

        Enemy* m_enemy;
        unsigned int m_awakeStateTimer;

        mono::IBodyPtr m_controlBody;
        mono::IConstraintPtr m_spring;
    };
}
