
#include "HomingBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include <cassert>

namespace tweak_values
{
    constexpr float block_detection_distance = 0.25f;
}

using namespace game;

HomingBehaviour::HomingBehaviour()
    : m_body(nullptr)
    , m_current_heading(0.0f)
    , m_forward_velocity(1.0f)
    , m_angular_velocity(180.0f)
    , m_homing_start_delay_s(0.0f)
    , m_homing_duration_s(math::INF)
    , m_sample_index(0)
    , m_timestamp(0)
{ }

void HomingBehaviour::SetBody(mono::IBody* body)
{
    m_body = body;
    m_current_heading = m_body->GetAngle();
}

void HomingBehaviour::SetHeading(float heading)
{
    m_current_heading = heading;
}

void HomingBehaviour::SetTargetPosition(const math::Vector& position)
{
    m_target_position = position;
    m_sample_index = 0;
}

const math::Vector& HomingBehaviour::GetTargetPosition() const
{
    return m_target_position;
}

void HomingBehaviour::SetForwardVelocity(float velocity)
{
    m_forward_velocity = velocity;
}

void HomingBehaviour::SetAngularVelocity(float degrees_per_second)
{
    m_angular_velocity = degrees_per_second;
}

void HomingBehaviour::SetHomingStartDelay(float delay_s)
{
    m_homing_start_delay_s = delay_s;
}

void HomingBehaviour::SetHomingDuration(float duration_s)
{
    m_homing_duration_s = duration_s;
}

HomingResult HomingBehaviour::Run(const mono::UpdateContext& update_context)
{
    const math::Vector body_position = m_body->GetPosition();
    const math::Vector& delta = math::Normalized(m_target_position - body_position);

    HomingResult result;
    result.distance_to_target = math::DistanceBetween(m_target_position, body_position);
    result.new_heading = m_current_heading;
    result.is_stuck = false;

    m_homing_start_delay_s -= update_context.delta_s;
    if(m_homing_start_delay_s > 0.0f)
        return result;

    if(m_homing_duration_s < 0.0f)
        return result;

    m_homing_duration_s -= update_context.delta_s;

    const math::Vector& vector_angle = math::VectorFromAngle(m_current_heading);
    const float dot_value = math::Dot(delta, vector_angle);
    const float cross_value = math::Cross(delta, vector_angle); // This gives the direction to turn in, i think.

    const float scaled_clamped_dot = math::Scale01Clamped(dot_value, 1.0f, 0.5f);
    const float scale_value = scaled_clamped_dot * ((cross_value < 0.0f) ? 1.0f : -1.0f);

    const float radians_turn = update_context.delta_s * math::ToRadians(m_angular_velocity);
    const float turn_value = scale_value * radians_turn;

    m_current_heading += turn_value;
    result.new_heading = m_current_heading;
    result.is_stuck = CheckIfBlocked(m_position_samples, update_context.timestamp, body_position);

    const math::Vector angle1 = math::VectorFromAngle(m_current_heading);
    m_body->SetVelocity(angle1 * m_forward_velocity);

/*
    const float mass = m_body->GetMass();
    const math::Vector angle1Norm = math::Normalized(angle1);
    const math::Vector mass_adjusted_impulse = angle1Norm * m_forward_velocity * mass * update_context.delta_s;
    m_body->ApplyLocalImpulse(mass_adjusted_impulse, math::ZeroVec);
*/

    return result;
}

bool HomingBehaviour::CheckIfBlocked(std::array<math::Vector, 10>& position_samples, uint32_t current_timestamp, const math::Vector& current_position)
{
    if(current_timestamp < m_timestamp + 500)
        return false;

    const int sample_index = m_sample_index % 10;
    m_sample_index++;

    m_position_samples[sample_index] = current_position;
    m_timestamp = current_timestamp;

    if(m_sample_index < 10)
        return false;

    bool blocked = true;
    const float block_detection_distance_sq =
        tweak_values::block_detection_distance * tweak_values::block_detection_distance;

    math::Vector center;
    for(const math::Vector& sample : position_samples)
        center += sample;

    center = center / position_samples.size();

    for(const math::Vector& sample : position_samples)
    {
        const float distance_to_center_sq = math::DistanceBetweenSquared(center, sample);
        if(distance_to_center_sq > block_detection_distance_sq)
        {
            blocked = false;
            break;
        }
    }

    return blocked;
}
