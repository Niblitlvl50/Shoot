
#include "BeastController.h"
#include "Enemy.h"
#include "EntityProperties.h"
#include "Rendering/Sprite/ISprite.h"

using namespace game;

BeastController::BeastController(mono::EventHandler& event_handler)
{
    using namespace std::placeholders;

    const std::unordered_map<BeastStates, BeastStateMachine::State>& state_table = {
        { BeastStates::IDLE,      { std::bind(&BeastController::ToIdle,      this), std::bind(&BeastController::Idle,      this, _1) } },
        { BeastStates::TRACKING,  { std::bind(&BeastController::ToTracking,  this), std::bind(&BeastController::Tracking,  this, _1) } },
        { BeastStates::ATTACKING, { std::bind(&BeastController::ToAttacking, this), std::bind(&BeastController::Attacking, this, _1) } }
    };

    m_states.SetStateTable(state_table);    
}

void BeastController::Initialize(Enemy* enemy)
{
    m_enemy = enemy;
    m_enemy->SetProperty(EntityProperties::DAMAGABLE);
    m_states.TransitionTo(BeastStates::IDLE);
}

void BeastController::doUpdate(unsigned int delta)
{
    m_states.UpdateState(delta);

    m_timer += delta;
}

void BeastController::ToIdle()
{
    m_enemy->m_sprite->SetAnimation(3);
    m_timer = 0;
}

void BeastController::ToTracking()
{
    m_enemy->m_sprite->SetAnimation(2);
    m_timer = 0;
}

void BeastController::ToAttacking()
{
    m_enemy->m_sprite->SetAnimation(1);
    m_timer = 0;
}

void BeastController::Idle(unsigned int delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::TRACKING);
}

void BeastController::Tracking(unsigned int delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::ATTACKING);
}

void BeastController::Attacking(unsigned int delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::IDLE);
}
