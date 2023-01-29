
#include "CirculatingBehaviour.h"

#include "IUpdatable.h"
#include "Math/MathFunctions.h"
#include "Math/Vector.h"
#include "Physics/IBody.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

CirculatingBehaviour::CirculatingBehaviour(mono::TransformSystem* transform_system)
    : m_transform_system(transform_system)
    , m_position_entity_id(-1)
    , m_body(nullptr)
    , m_forward_velocity(2.0f)
    , m_angular_velocity(160.0f)
    , m_radius(1.0f)
    , m_current_angle_rad(0.0f)
{ }

void CirculatingBehaviour::Initialize(uint32_t position_entity_id, float radius, float heading, mono::IBody* body)
{
    m_position_entity_id = position_entity_id;
    m_radius = radius;
    m_current_angle_rad = heading;
    m_body = body;
    m_forward_velocity = math::Length(body->GetVelocity());

    //
    // https://en.wikipedia.org/wiki/Angular_velocity
    // ...and then go slightly faster.
    //
    m_angular_velocity = (m_forward_velocity / m_radius) * 1.1f;
}

void CirculatingBehaviour::Run(const mono::UpdateContext& update_context)
{
    const math::Vector unit_rotation = math::VectorFromAngle(m_current_angle_rad);
    const math::Vector radius_position = unit_rotation * m_radius;

    m_current_angle_rad =
        math::NormalizeAngle(m_current_angle_rad + (update_context.delta_s * m_angular_velocity));

    const math::Vector center_position = m_transform_system->GetWorldPosition(m_position_entity_id);
    const math::Vector delta_to_position = (center_position + radius_position) - m_body->GetPosition();
    m_body->SetVelocity(math::Normalized(delta_to_position) * m_forward_velocity);
}
