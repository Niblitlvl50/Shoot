
#include "TrackingBehaviour.h"
#include "Navigation/NavigationSystem.h"

#include "Math/CriticalDampedSpring.h"
#include "Paths/IPath.h"
#include "Paths/PathFactory.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "Physics/PhysicsSystem.h"
#include "System/Debug.h"

using namespace game;

TrackingBehaviour::TrackingBehaviour()
    : m_tracking_position(math::INF, math::INF)
    , m_current_position(0.0f)
    , m_meter_per_second(1.0f)
    , m_timestamp_last_updated(0.0f)
{ }

TrackingBehaviour::~TrackingBehaviour() = default;

void TrackingBehaviour::Init(mono::IBody* body, NavigationSystem* navigation_system)
{
    MONO_ASSERT(body->GetType() == mono::BodyType::DYNAMIC);

    m_entity_body = body;
    m_navigation_system = navigation_system;
    m_tracking_position = body->GetPosition();
}

void TrackingBehaviour::SetTrackingSpeed(float meter_per_second)
{
    m_meter_per_second = meter_per_second;
}

TrackingResult TrackingBehaviour::Run(const mono::UpdateContext& update_context, const math::Vector& tracking_position)
{
    TrackingResult result;
    result.state = TrackingState::NO_PATH;
    result.distance_to_target = math::INF;

    const float delta_time = update_context.timestamp - m_timestamp_last_updated;

    const float distance_to_last = math::DistanceBetween(m_tracking_position, tracking_position);
    const bool time_to_update_path = distance_to_last > 0.5f && delta_time > 2.0f;

    if(!m_path || time_to_update_path)
    {
        m_timestamp_last_updated = update_context.timestamp;

        const bool path_updated = UpdatePath(tracking_position);
        if(!path_updated)
            return result;

        m_tracking_position = tracking_position;
    }

    result.distance_to_target = m_path->Length() - m_current_position;

    if(result.distance_to_target <= 0.0f)
    {
        result.state = TrackingState::AT_TARGET;
    }
    else
    {
        m_current_position += m_meter_per_second * update_context.delta_s;
        math::Vector current_position = m_entity_body->GetPosition();
        const math::Vector& path_position = m_path->GetPositionByLength(m_current_position);

        constexpr float move_halflife = 0.3f;

        math::critical_spring_damper(
            current_position,
            m_move_velocity,
            path_position,
            math::ZeroVec,
            move_halflife,
            update_context.delta_s);

        m_entity_body->SetVelocity(m_move_velocity);

        //const math::Vector mass_adjusted_impulse = m_move_velocity * m_entity_body->GetMass() * update_context.delta_s;
        //m_entity_body->ApplyLocalImpulse(mass_adjusted_impulse, math::ZeroVec);
        
        result.state = TrackingState::TRACKING;
    }

    return result;
}

bool TrackingBehaviour::UpdatePath(const math::Vector& tracking_position)
{
    const math::Vector position = m_entity_body->GetPosition();
    const std::vector<math::Vector>& found_path = m_navigation_system->FindPath(position, tracking_position);
    if(found_path.empty())
        return false;

    m_path = mono::CreatePath(found_path);
    m_current_position = m_path->GetLengthFromPosition(position);

    return true;
}

const math::Vector& TrackingBehaviour::GetTrackingPosition() const
{
    return m_tracking_position;
}

TrackingDebugData TrackingBehaviour::GetDebugData() const
{
    return { m_current_position, m_timestamp_last_updated, m_meter_per_second };
}
