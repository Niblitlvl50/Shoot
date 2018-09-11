
#pragma once

#include "MonoPtrFwd.h"
#include "Physics/IBody.h"
#include "Enemy.h"
#include "StateMachine.h"

namespace game
{
    class BlackSquareController : public IEnemyController, public mono::ICollisionHandler
    {
    public:

        BlackSquareController(float trigger_distance, mono::EventHandler& event_handler);
        virtual ~BlackSquareController();

        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;
        void OnCollideWith(const mono::IBodyPtr& body, unsigned int category) override;

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

        const float m_trigger_distance;
        mono::EventHandler& m_event_handler;
        unsigned int m_awake_state_timer;

        using MyStateMachine = StateMachine<States, unsigned int>;
        MyStateMachine m_states;

        Enemy* m_enemy;
        std::unique_ptr<class HomingBehaviour> m_homing_behaviour;
    };
}
