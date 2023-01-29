
#include "SineWaveBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"

#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

#include <cassert>

using namespace game;

SineWaveBehaviour::SineWaveBehaviour()
    : m_body(nullptr)
    , m_radians(0.0f)
    , m_sine_speed_deg_s(90.0f)
    , m_magnitude(0.5f)
{ }

SineWaveBehaviour::SineWaveBehaviour(mono::IBody* body)
    : SineWaveBehaviour()
{
    SetBody(body);
}

void SineWaveBehaviour::SetBody(mono::IBody* body)
{
    m_body = body;
}

void SineWaveBehaviour::SetHeading(float heading)
{
}

void SineWaveBehaviour::Run(const mono::UpdateContext& update_context)
{
    m_radians += (update_context.delta_s * math::ToRadians(m_sine_speed_deg_s));

    const float sine_value = std::sin(m_radians);

    const math::Vector velocity = m_body->GetVelocity();
    const math::Vector velocity_normalized = math::Normalized(velocity);
    const math::Vector perpendicular_to_vel = math::Perpendicular(velocity_normalized);

    const math::Vector current_position = m_body->GetPosition();
    const math::Vector m_sine_position = current_position + (perpendicular_to_vel * sine_value * m_magnitude);

    g_debug_drawer->DrawPointFading(m_sine_position, 5.0f, mono::Color::MAGENTA, 1.0f);

    //const math::Vector velocity_add = perpendicular_to_vel * m_magnitude * (signbit ? 1.0f : -1.0f);
    //m_body->SetVelocity(velocity + velocity_add);
}
