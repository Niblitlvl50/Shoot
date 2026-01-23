
#include "PathBehaviour.h"

#include "Math/CriticalDampedSpring.h"
#include "Paths/IPath.h"
#include "Physics/IBody.h"
#include "System/Debug.h"

using namespace game;

PathBehaviour::PathBehaviour()
{ }

PathBehaviour::PathBehaviour(mono::IBody* entity_body, mono::IPathPtr path)
{
    Init(entity_body);
    SetPath(std::move(path));
}

PathBehaviour::~PathBehaviour()
{ }

void PathBehaviour::Init(mono::IBody* entity_body)
{
    m_entity_body = entity_body;
}

void PathBehaviour::SetPath(mono::IPathPtr path)
{
    m_path = std::move(path);
}

void PathBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

PathResult PathBehaviour::Run(float delta_s)
{
    PathResult result;
    result.distance_to_target = math::INF;
    result.is_stuck = false;

    if(!m_path)
        return result;

    m_current_position += m_meter_per_second * delta_s;
    math::Vector current_position = m_entity_body->GetPosition();
    const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);

    constexpr float move_halflife = 0.3f;

    math::critical_spring_damper(
        current_position,
        m_move_velocity,
        path_position,
        math::ZeroVec,
        move_halflife,
        delta_s);

    m_entity_body->SetVelocity(m_move_velocity);

    //const math::Vector mass_adjusted_impulse = m_move_velocity * m_entity_body->GetMass() * update_context.delta_s;
    //m_entity_body->ApplyLocalImpulse(mass_adjusted_impulse, math::ZeroVec);

    return result;
}