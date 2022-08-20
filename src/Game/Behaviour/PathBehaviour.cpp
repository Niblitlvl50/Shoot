
#include "PathBehaviour.h"

#include "Paths/IPath.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "Physics/PhysicsSystem.h"
#include "System/Debug.h"


using namespace game;

PathBehaviour::PathBehaviour(
    mono::IBody* entity_body, const mono::IPath* path, mono::PhysicsSystem* physics_system)
    : m_path(path)
    , m_physics_system(physics_system)
    , m_current_position(0.0f)
    , m_meter_per_second(1.0f)
{
    MONO_ASSERT(entity_body->GetType() == mono::BodyType::DYNAMIC);
    m_control_body = m_physics_system->CreateKinematicBody();
    m_spring = m_physics_system->CreateSpring(m_control_body, entity_body, 0.0f, 200.0f, 10.0f);
}

PathBehaviour::~PathBehaviour()
{
    m_physics_system->ReleaseConstraint(m_spring);
    m_physics_system->ReleaseKinematicBody(m_control_body);
}

void PathBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

void PathBehaviour::Run(const mono::UpdateContext& update_context)
{
    m_current_position += m_meter_per_second * update_context.delta_s;

    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);
    m_control_body->SetPosition(path_position);

    if(m_current_position > m_path->Length())
        m_current_position = 0.0f;
}
