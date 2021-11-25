
#include "HomingBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include <cassert>

using namespace game;

HomingBehaviour::HomingBehaviour()
    : m_body(nullptr)
    , m_current_heading(0.0f)
    , m_forward_velocity(0.1f)
    , m_angular_velocity(180.0f)
{ }

HomingBehaviour::HomingBehaviour(mono::IBody* body)
    : HomingBehaviour()
{
    SetBody(body);
}

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
}

void HomingBehaviour::SetForwardVelocity(float velocity)
{
    m_forward_velocity = velocity;
}

void HomingBehaviour::SetAngularVelocity(float degrees_per_second)
{
    m_angular_velocity = degrees_per_second;
}

HomingResult HomingBehaviour::Run(const mono::UpdateContext& update_context)
{
    const math::Vector& entity_position = m_body->GetPosition();
    const math::Vector& delta = math::Normalized(m_target_position - entity_position);

    const math::Vector& vector_angle = math::VectorFromAngle(m_current_heading);

    const float dot_value = math::Dot(delta, vector_angle);
    const float cross_value = math::Cross(delta, vector_angle);

    const float scaled_clamped_dot = math::Scale01Clamped(dot_value, 1.0f, 0.5f);
    const float scale_value = scaled_clamped_dot * ((cross_value < 0.0f) ? 1.0f : -1.0f);

    const float radians_turn = update_context.delta_s * math::ToRadians(m_angular_velocity);
    const float turn_value = scale_value * radians_turn;

    m_current_heading += turn_value;

    const math::Vector angle1 = math::VectorFromAngle(m_current_heading);
    m_body->ApplyLocalImpulse(angle1 * m_forward_velocity * update_context.delta_s, math::ZeroVec);

    return { m_current_heading, math::DistanceBetween(m_target_position, entity_position) };
}
