
#include "HomingBehaviour.h"

#include "Enemies/Enemy.h"
#include "Events/SpawnConstraintEvent.h"

#include "EventHandler/EventHandler.h"
#include "Physics/CMFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"

using namespace game;

HomingBehaviour::HomingBehaviour(Enemy* enemy, mono::EventHandler& event_handler)
    : m_event_handler(event_handler)
{
    m_control_body = mono::PhysicsFactory::CreateKinematicBody();
    m_control_body->SetPosition(enemy->Position());

    m_spring = mono::PhysicsFactory::CreateSpring(m_control_body, enemy->GetPhysics().body, 0.0f, 50.0f, 0.5f);
    m_event_handler.DispatchEvent(SpawnConstraintEvent(m_spring));
}

HomingBehaviour::~HomingBehaviour()
{
    m_event_handler.DispatchEvent(DespawnConstraintEvent(m_spring));
}

void HomingBehaviour::SetHomingPosition(const math::Vector& position)
{
    m_control_body->SetPosition(position);
}
