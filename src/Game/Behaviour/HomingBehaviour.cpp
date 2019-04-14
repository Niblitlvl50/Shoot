
#include "HomingBehaviour.h"

#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"

#include "Math/Vector.h"

#include <cassert>

using namespace game;

HomingBehaviour::HomingBehaviour(mono::IBody* body, mono::PhysicsSystem* physics_system)
    : m_physics_system(physics_system)
{
    assert(body->GetType() == mono::BodyType::DYNAMIC);

    m_control_body = m_physics_system->CreateKinematicBody();
    m_control_body->SetPosition(body->GetPosition());
    m_spring = m_physics_system->CreateSpring(body, m_control_body, 0.0f, 50.0f, 0.5f);
}

HomingBehaviour::~HomingBehaviour()
{
    m_physics_system->ReleaseConstraint(m_spring);
    m_physics_system->ReleaseKinematicBody(m_control_body);
}

void HomingBehaviour::SetHomingPosition(const math::Vector& position)
{
    m_control_body->SetPosition(position);
}
