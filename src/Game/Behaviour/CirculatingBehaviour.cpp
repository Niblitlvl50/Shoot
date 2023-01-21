
#include "CirculatingBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include <cassert>

using namespace game;

CirculatingBehaviour::CirculatingBehaviour()
    : m_body(nullptr)
    , m_forward_velocity(5.0f)
    , m_angular_velocity(180.0f)
    , m_current_angle_rad(0.0f)
{ }

CirculatingBehaviour::CirculatingBehaviour(mono::IBody* body)
    : CirculatingBehaviour()
{
    SetBody(body);
}

void CirculatingBehaviour::SetBody(mono::IBody* body)
{
    m_body = body;
}

void CirculatingBehaviour::SetForwardVelocity(float velocity)
{
    m_forward_velocity = velocity;
}

void CirculatingBehaviour::SetAngularVelocity(float degrees_per_second)
{
    m_angular_velocity = degrees_per_second;
}

void CirculatingBehaviour::Run(const mono::UpdateContext& update_context)
{
    const math::Vector unit_rotation = math::VectorFromAngle(m_current_angle_rad);
    const math::Vector perpendicular_vector = math::Perpendicular(unit_rotation);

    m_current_angle_rad =
        math::NormalizeAngle(m_current_angle_rad + (update_context.delta_s * math::ToRadians(m_angular_velocity)));

    m_body->SetVelocity(perpendicular_vector * m_forward_velocity);
}
