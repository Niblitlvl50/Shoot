
#include "BombThrowerController.h"

using namespace game;

BombThrowerController::BombThrowerController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
{
    const TStateMachine::StateTable state_table = {
        TStateMachine::MakeState(States::IDLE, &BombThrowerController::EnterIdle, &BombThrowerController::Idle, this),
        TStateMachine::MakeState(States::ATTACK, &BombThrowerController::EnterAttack, &BombThrowerController::Attack, this),
    };
    m_states.SetStateTableAndState(state_table, States::IDLE);
}

void BombThrowerController::Update(const mono::UpdateContext& update_context)
{
    m_states.UpdateState(update_context);
}

void BombThrowerController::DrawDebugInfo(class IDebugDrawer* debug_drawer) const
{

}

const char* BombThrowerController::GetDebugCategory() const
{
    return "BombThrower";
}

void BombThrowerController::EnterIdle()
{

}

void BombThrowerController::Idle(const mono::UpdateContext& update_context)
{

}

void BombThrowerController::EnterAttack()
{

}

void BombThrowerController::Attack(const mono::UpdateContext& update_context)
{

}
