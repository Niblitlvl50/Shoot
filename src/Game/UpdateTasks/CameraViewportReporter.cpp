
#include "CameraViewportReporter.h"
#include "Camera/ICamera.h"
#include "AIKnowledge.h"

#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"

#include "TransformSystem/TransformSystem.h"
#include "Math/MathFunctions.h"

#include <cmath>
#include <limits>

namespace
{
    constexpr const uint32_t NO_ENTITY = std::numeric_limits<uint32_t>::max();
}

using namespace game;

GameCamera::GameCamera(mono::ICamera* camera, mono::TransformSystem* transform_system, mono::EventHandler& event_handler)
    : m_camera(camera)
    , m_transform_system(transform_system)
    , m_event_handler(event_handler)
    , m_controller(camera, event_handler)
    , m_debug_camera(false)
    , m_entity_id(NO_ENTITY)
{
    const event::KeyDownEventFunc key_down_func = [this](const event::KeyDownEvent& event) {
        const bool toggle_camera = (event.key == Keycode::D);
        if(toggle_camera)
        {
            m_debug_camera = !m_debug_camera;
            m_debug_camera ? m_controller.Enable() : m_controller.Disable();
        }

        return mono::EventResult::PASS_ON;
    };

    m_key_down_token = m_event_handler.AddListener(key_down_func);
}

GameCamera::~GameCamera()
{
    m_event_handler.RemoveListener(m_key_down_token);
}

void GameCamera::doUpdate(const mono::UpdateContext& update_context)
{
    const math::Quad& viewport = m_camera->GetViewport();
    g_camera_viewport = math::Quad(viewport.mA, viewport.mA + viewport.mB);

    if(m_entity_id != NO_ENTITY)
    {
        const math::Matrix& transform = m_transform_system->GetTransform(m_entity_id);

        const float rotation = math::GetZRotation(transform);
        const math::Vector& position = math::GetPosition(transform);

        math::Quad viewport = m_camera->GetViewport();
        const float ratio_value = viewport.mB.y / viewport.mB.x;

        const math::Vector ratio(1.0f, ratio_value);
        const math::Vector& xy = -math::VectorFromAngle(rotation) * ratio * 3.0f;

        const math::Vector& target_position = position - (viewport.mB * 0.5f) - xy;
        const math::Vector& diff = target_position - viewport.mA;

        constexpr float SPEED = 0.005f;
        const math::Vector& move = diff * (update_context.delta_ms * SPEED);
        viewport.mA += move;

        m_camera->SetViewport(viewport);
    }
}

void GameCamera::Follow(uint32_t entity_id, const math::Vector& offset)
{
    m_entity_id = entity_id;
    m_offset = offset;
}

void GameCamera::Unfollow()
{
    m_entity_id = NO_ENTITY;
}
