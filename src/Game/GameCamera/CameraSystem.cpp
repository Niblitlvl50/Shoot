
#include "CameraSystem.h"
#include "TriggerSystem.h"
#include "AIKnowledge.h"

#include "Camera/ICamera.h"
#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"
#include "Math/MathFunctions.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"
#include "Util/Hash.h"

#include <cmath>
#include <limits>

namespace
{
    constexpr const uint32_t NO_ENTITY = std::numeric_limits<uint32_t>::max();
}

using namespace game;

CameraSystem::CameraSystem(
    uint32_t n, mono::ICamera* camera, mono::TransformSystem* transform_system, mono::EventHandler* event_handler, TriggerSystem* trigger_system)
    : m_camera(camera)
    , m_transform_system(transform_system)
    , m_event_handler(event_handler)
    , m_trigger_system(trigger_system)
    , m_controller(camera, *event_handler)
    , m_debug_camera(false)
    , m_entity_id(NO_ENTITY)
{
    m_camera_components.resize(n);
    m_active_camera_components.resize(n, false);

    const event::KeyDownEventFunc key_down_func = [this](const event::KeyDownEvent& event) {
        const bool toggle_camera = (event.key == Keycode::D);
        if(toggle_camera)
        {
            m_debug_camera = !m_debug_camera;
            m_debug_camera ? m_controller.Enable() : m_controller.Disable();
        }

        return mono::EventResult::PASS_ON;
    };

    m_key_down_token = m_event_handler->AddListener(key_down_func);
}

CameraSystem::~CameraSystem()
{
    m_event_handler->RemoveListener(m_key_down_token);
}

uint32_t CameraSystem::Id() const
{
    return mono::Hash(Name());
}

const char* CameraSystem::Name() const
{
    return "camerasystem";
}

void CameraSystem::Update(const mono::UpdateContext& update_context)
{
    const math::Quad& viewport = m_camera->GetViewport();
    g_camera_viewport = math::Quad(viewport.mA, viewport.mA + viewport.mB);

    if(m_entity_id != NO_ENTITY)
    {
        const math::Matrix& world_transform = m_transform_system->GetWorld(m_entity_id);
        const math::Vector& position = math::GetPosition(world_transform);
        m_camera->SetTargetPosition(position);
    }

    const auto process_camera_anims = [this, &update_context](CameraAnimationComponent* camera_anim)
    {
        switch(camera_anim->type)
        {
        case CameraAnimationType::ZOOM_LEVEL:
        {
            const math::Vector viewport_size = m_camera->GetViewportSize();
            const math::Vector new_viewport_size = viewport_size * camera_anim->zoom_level;
            m_camera->SetTargetViewportSize(new_viewport_size);
            break;
        }
        case CameraAnimationType::CENTER_ON_POINT:
            m_camera->SetTargetPosition(math::Vector(camera_anim->point_x, camera_anim->point_y));
            break;
        case CameraAnimationType::CENTER_ON_ENTITY:
            break;
        };

        return true;
    };

    mono::remove_if(m_camera_anims_to_process, process_camera_anims);
}

void CameraSystem::Follow(uint32_t entity_id, const math::Vector& offset)
{
    m_entity_id = entity_id;
    //m_offset = offset;
}

void CameraSystem::Unfollow()
{
    m_entity_id = NO_ENTITY;
}

CameraAnimationComponent* CameraSystem::AllocateCameraAnimation(uint32_t entity_id)
{
    m_active_camera_components[entity_id] = true;
    CameraAnimationComponent* allocated_component = &m_camera_components[entity_id];
    std::memset(allocated_component, 0, sizeof(CameraAnimationComponent));
    return allocated_component;
}

void CameraSystem::ReleaseCameraAnimation(uint32_t entity_id)
{
    m_active_camera_components[entity_id] = false;
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, float new_zoom_level)
{
    CameraAnimationComponent* camera_component = &m_camera_components[entity_id];
    camera_component->trigger_hash = trigger_hash;
    camera_component->type = CameraAnimationType::ZOOM_LEVEL;
    camera_component->zoom_level = new_zoom_level;

    const TriggerCallback callback = [this, camera_component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(camera_component);
    };

    camera_component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, const math::Vector& world_point)
{
    CameraAnimationComponent* camera_component = &m_camera_components[entity_id];
    camera_component->trigger_hash = trigger_hash;
    camera_component->type = CameraAnimationType::CENTER_ON_POINT;
    camera_component->point_x = world_point.x;
    camera_component->point_y = world_point.y;

    const TriggerCallback callback = [this, camera_component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(camera_component);
    };

    camera_component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t follow_entity_id)
{
    CameraAnimationComponent* camera_component = &m_camera_components[entity_id];
    camera_component->trigger_hash = trigger_hash;
    camera_component->type = CameraAnimationType::CENTER_ON_ENTITY;
    camera_component->entity_id = follow_entity_id;

    const TriggerCallback callback = [this, camera_component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(camera_component);
    };

    camera_component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}
