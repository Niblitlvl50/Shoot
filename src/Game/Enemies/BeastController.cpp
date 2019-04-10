
#include "BeastController.h"
#include "Rendering/Sprite/ISprite.h"
#include "Rendering/Sprite/Sprite.h"
#include "Rendering/Sprite/SpriteSystem.h"

#include "SystemContext.h"

using namespace game;

BeastController::BeastController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
{
    mono::SpriteSystem* sprite_system = system_context->GetSystem<mono::SpriteSystem>();
    m_sprite = sprite_system->GetSprite(entity_id);

    using namespace std::placeholders;

    const std::unordered_map<BeastStates, BeastStateMachine::State>& state_table = {
        { BeastStates::IDLE,      { std::bind(&BeastController::ToIdle,      this), std::bind(&BeastController::Idle,      this, _1) } },
        { BeastStates::TRACKING,  { std::bind(&BeastController::ToTracking,  this), std::bind(&BeastController::Tracking,  this, _1) } },
        { BeastStates::ATTACKING, { std::bind(&BeastController::ToAttacking, this), std::bind(&BeastController::Attacking, this, _1) } }
    };

    m_states.SetStateTable(state_table);
    m_states.TransitionTo(BeastStates::IDLE);
}

void BeastController::Update(uint32_t delta_ms)
{
    m_states.UpdateState(delta_ms);
    m_timer += delta_ms;
}

void BeastController::ToIdle()
{
    m_sprite->SetAnimation(3);
    m_timer = 0;
}

void BeastController::ToTracking()
{
    m_sprite->SetAnimation(2);
    m_timer = 0;
}

void BeastController::ToAttacking()
{
    m_sprite->SetAnimation(1);
    m_timer = 0;
}

void BeastController::Idle(uint32_t delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::TRACKING);
}

void BeastController::Tracking(uint32_t delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::ATTACKING);
}

void BeastController::Attacking(uint32_t delta)
{
    if(m_timer > 1000)
        m_states.TransitionTo(BeastStates::IDLE);
}
