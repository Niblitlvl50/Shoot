
#include "PathBehaviour.h"

#include "Math/CriticalDampedSpring.h"
#include "Math/MathFunctions.h"
#include "Paths/IPath.h"
#include "Physics/IBody.h"
#include "System/Debug.h"

#include <algorithm>

using namespace game;

PathBehaviour::PathBehaviour()
{ }

PathBehaviour::PathBehaviour(mono::IBody* entity_body, mono::IPathPtr path)
{
    Init(entity_body);
    SetPath(std::move(path));
}

PathBehaviour::PathBehaviour(PathBehaviour&& other) noexcept = default;
PathBehaviour& PathBehaviour::operator=(PathBehaviour&& other) noexcept = default;

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

void PathBehaviour::SetPingPong(bool ping_pong)
{
    m_ping_pong = ping_pong;
}

void PathBehaviour::SetLoop(bool loop)
{
    m_loop = loop;
}

void PathBehaviour::SetOffset(const math::Vector& offset)
{
    m_offset = offset;
}

void PathBehaviour::SetPaused(bool paused)
{
    m_paused = paused;
}

void PathBehaviour::SetManualControl(bool manual_control)
{
    m_manual_control = manual_control;
}

void PathBehaviour::SetThrottle(float throttle)
{
    m_throttle = std::clamp(throttle, -1.0f, 1.0f);
}

float PathBehaviour::GetThrottle() const
{
    return m_throttle;
}

void PathBehaviour::SetCurrentPosition(float position)
{
    m_current_position = position;
}

void PathBehaviour::TeleportToPosition(float position)
{
    m_current_position = position;

    if(m_entity_body && m_path)
    {
        const mono::PositionResult pos_result = m_path->GetPositionByLength(m_current_position);
        if(pos_result.valid_position)
            m_entity_body->SetPosition(pos_result.path_position + m_offset);
    }
}

float PathBehaviour::GetCurrentPosition() const
{
    return m_current_position;
}

float PathBehaviour::GetPathLength() const
{
    return m_path ? m_path->Length() : 0.0f;
}

const std::vector<math::Vector>* PathBehaviour::GetPathPoints() const
{
    return m_path ? &m_path->GetPathPoints() : nullptr;
}

float PathBehaviour::GetCurvature() const
{
    return m_path ? m_path->GetCurvatureByLength(m_current_position) : 0.0f;
}

PathResult PathBehaviour::Run(float delta_s)
{
    PathResult result;
    result.distance_to_target = math::INF;
    result.is_stuck = false;

    if(!m_path)
        return result;

    if(m_paused)
    {
        m_entity_body->SetVelocity(math::ZeroVec);
    }
    else
    {
        if(m_manual_control)
        {
            m_current_position += m_meter_per_second * m_throttle * delta_s;
            m_current_position = std::clamp(m_current_position, 0.0f, m_path->Length());

            if(m_throttle != 0.0f)
                m_direction = (m_throttle > 0.0f) ? 1.0f : -1.0f;
        }
        else
        {
            m_current_position += m_meter_per_second * m_direction * delta_s;

            if(m_ping_pong)
            {
                if(m_current_position >= m_path->Length())
                {
                    m_current_position = m_path->Length();
                    m_direction = -1.0f;
                }
                else if(m_current_position <= 0.0f)
                {
                    m_current_position = 0.0f;
                    m_direction = 1.0f;
                }
            }
            else if(m_loop && m_current_position >= m_path->Length())
            {
                m_current_position = 0.0f;
            }
        }

        math::Vector current_position = m_entity_body->GetPosition();
        const mono::PositionResult position_result = m_path->GetPositionByLength(m_current_position);
        if(position_result.valid_position)
        {
            constexpr float move_halflife = 0.1f;
            const math::Vector target_position = position_result.path_position + m_offset;

            math::critical_spring_damper(
                current_position,
                m_move_velocity,
                target_position,
                math::ZeroVec,
                move_halflife,
                delta_s);

            m_entity_body->SetVelocity(m_move_velocity);

            if(m_apply_rotation)
            {
                const math::Vector tangent = m_path->GetTangentByLength(m_current_position) * m_direction;
                m_entity_body->SetAngle(math::AngleFromVector(tangent));
            }
        }
    }

    const mono::LengthResult length_result = m_path->GetLengthFromPosition(m_entity_body->GetPosition());
    if(length_result.valid_length)
    {
        result.distance_to_target = m_path->Length() - length_result.path_length;
    }

    //const math::Vector mass_adjusted_impulse = m_move_velocity * m_entity_body->GetMass() * update_context.delta_s;
    //m_entity_body->ApplyLocalImpulse(mass_adjusted_impulse, math::ZeroVec);

    return result;
}

void PathBehaviour::SetApplyRotation(bool apply_rotation)
{
    m_apply_rotation = apply_rotation;
}

PathDebugData PathBehaviour::GetDebugData() const
{
    PathDebugData data;
    data.has_path = (m_path != nullptr);
    data.path_points = data.has_path ? &m_path->GetPathPoints() : nullptr;
    data.target_position = math::ZeroVec;

    if(data.has_path)
    {
        const mono::PositionResult pos_result = m_path->GetPositionByLength(m_current_position);
        if(pos_result.valid_position)
            data.target_position = pos_result.path_position + m_offset;
    }

    return data;
}