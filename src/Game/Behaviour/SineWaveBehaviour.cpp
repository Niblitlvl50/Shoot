
#include "SineWaveBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include <cassert>

using namespace game;

SineWaveBehaviour::SineWaveBehaviour()
    : m_body(nullptr)
    , m_radians(0.0f)
    , m_sine_speed_deg_s(360.0f)
    , m_magnitude(0.35f)
{ }

void SineWaveBehaviour::Initialize(mono::IBody* body, const math::Vector& position)
{
    m_body = body;
    m_position = position;
    m_velocity = m_body->GetVelocity();
    m_body->SetVelocity(math::ZeroVec);

    static bool s_flip_flop = false;
    m_sine_speed_deg_s *= s_flip_flop ? 1.0f : -1.0f;
    s_flip_flop = !s_flip_flop;
}

void SineWaveBehaviour::Run(const mono::UpdateContext& update_context)
{
    m_radians += (update_context.delta_s * math::ToRadians(m_sine_speed_deg_s));
    const float sine_value = std::sin(m_radians);

    m_position += (m_velocity * update_context.delta_s);

    const math::Vector velocity_normalized = math::Normalized(m_velocity);
    const math::Vector perpendicular_to_vel = math::Perpendicular(velocity_normalized);

    const math::Vector m_sine_position = m_position + (perpendicular_to_vel * sine_value * m_magnitude);
    m_body->SetPosition(m_sine_position);
}
