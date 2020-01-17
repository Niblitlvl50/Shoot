
#include "Camera.h"
#include "Zone/IEntity.h"
#include "Math/Vector.h"
#include "Math/MathFunctions.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"

#include "SystemContext.h"
#include "TransformSystem/TransformSystem.h"

#include <cmath>
#include <limits>

using namespace game;

namespace
{
    constexpr const uint32_t NO_ENTITY = std::numeric_limits<uint32_t>::max();
}

Camera::Camera(int width, int height, int window_width, int window_height, mono::TransformSystem* transform_system, mono::EventHandler& event_handler)
    : m_offset(math::ZeroVec)
    , m_viewport(0.0f, 0.0f, width, height)
    , m_targetViewport(m_viewport)
    , m_controller(window_width, window_height, this, event_handler)
    , m_transform_system(transform_system)
    , m_event_handler(event_handler)
    , m_debug_camera(false)
    , m_entity_id(NO_ENTITY)
{
    using namespace std::placeholders;

    const event::KeyDownEventFunc key_down_func = std::bind(&Camera::OnKeyDown, this, _1);
    m_key_down_token = m_event_handler.AddListener(key_down_func);
}

Camera::~Camera()
{
    m_event_handler.RemoveListener(m_key_down_token);
}

bool Camera::OnKeyDown(const event::KeyDownEvent& event)
{
    const bool toggle_camera = (event.key == Keycode::D);
    if(toggle_camera)
    {
        m_debug_camera = !m_debug_camera;
        m_debug_camera ? m_controller.Enable() : m_controller.Disable();
    }

    return false;
}

void Camera::doUpdate(const mono::UpdateContext& update_context)
{
    constexpr float SPEED = 0.005f;
    
    const float change = (m_targetViewport.mB.x - m_viewport.mB.x);
    const float xzero = std::floor(std::abs(change));

    if(xzero != 0.0f)
    {
        const float aspect = m_viewport.mB.x / m_viewport.mB.y;
        math::ResizeQuad(m_viewport, change * 0.1f, aspect);
    }

    if(m_debug_camera)
        return;
    
    if(m_entity_id != NO_ENTITY)
    {
        const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);

        const float rotation = math::GetZRotation(transform);
        const math::Vector& position = math::GetPosition(transform);

        const float ratio_value = m_viewport.mB.y / m_viewport.mB.x;

        const math::Vector ratio(1.0f, ratio_value);
        const math::Vector& xy = -math::VectorFromAngle(rotation) * ratio * 3.0f;

        const math::Vector& targetPosition = position - (m_viewport.mB * 0.5f) - xy;
        const math::Vector& diff = targetPosition - m_viewport.mA;
    
        const math::Vector& move = diff * (update_context.delta_ms * SPEED);
        m_viewport.mA += move;
    }
}

void Camera::Follow(uint32_t entity_id, const math::Vector& offset)
{
    m_entity_id = entity_id;
    m_offset = offset;
}

void Camera::Unfollow()
{
    m_entity_id = NO_ENTITY;
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
