
#include "Camera.h"
#include "Entity/IEntity.h"
#include "Math/Vector.h"
#include "Math/MathFunctions.h"

#include <cmath>

using namespace game;

Camera::Camera(int width, int height)
    : m_offset(math::zeroVec),
      m_viewport(0.0f, 0.0f, width, height),
      m_targetViewport(m_viewport)
{ }

void Camera::doUpdate(unsigned int delta)
{
    constexpr float SPEED = 0.004f;
    
    const float change = (m_targetViewport.mB.x - m_viewport.mB.x);
    const float xzero = std::floor(std::abs(change));

    if(xzero != 0.0f)
    {
        const float aspect = m_viewport.mB.x / m_viewport.mB.y;
        math::ResizeQuad(m_viewport, change * 0.1f, aspect);
    }
    
    if(m_entity)
    {
        const float rotation = m_entity->Rotation();

        const float ratio_value = m_viewport.mB.y / m_viewport.mB.x;

        const math::Vector ratio(1.0f, ratio_value);
        const math::Vector& xy = -math::VectorFromAngle(rotation) * ratio * 6.0f;

        const math::Vector& targetPosition = m_entity->Position() - (m_viewport.mB * 0.5f) - xy;
        const math::Vector& diff = targetPosition - m_viewport.mA;
    
        const math::Vector& move = diff * (delta * SPEED);
        m_viewport.mA += move;
    }
}

void Camera::Follow(const mono::IEntityPtr& entity, const math::Vector& offset)
{
    m_entity = entity;
    m_offset = offset;
}

void Camera::Unfollow()
{
    m_entity = nullptr;    
}

math::Quad Camera::GetViewport() const
{
    math::Quad result = m_viewport;
    result.mA -= m_offset;

    return result;
}

math::Vector Camera::GetPosition() const
{
    return m_viewport.mA + (m_viewport.mB * 0.5f);    
}

void Camera::SetViewport(const math::Quad& viewport)
{
    m_viewport = viewport;
    m_targetViewport = viewport;    
}

void Camera::SetTargetViewport(const math::Quad& target)
{
    m_targetViewport = target;    
}

void Camera::SetPosition(const math::Vector& position)
{
    // The position is the middle of the quad, not the lower left corner.
    const math::Vector& xy = position - (m_viewport.mB * 0.5f);
    m_viewport.mA = xy;
}

math::Vector Camera::ScreenToWorld(const math::Vector& screen_pos, const math::Vector& window_size) const
{
    const math::Quad& viewport = GetViewport();
    
    const math::Vector& scale = viewport.mB / window_size;
    
    const float screenX = screen_pos.x;
    const float screenY = window_size.y - screen_pos.y;
    
    const float tempx = screenX * scale.x;
    const float tempy = screenY * scale.y;
    
    return math::Vector(tempx + viewport.mA.x, tempy + viewport.mA.y);    
}
