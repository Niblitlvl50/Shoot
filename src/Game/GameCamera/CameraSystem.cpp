
#include "CameraSystem.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

// Debug
#include "GameDebug.h"
#include "Factories.h"
#include "IDebugDrawer.h"
#include "Rendering/Color.h"

#include "Camera/ICamera.h"
#include "EventHandler/EventHandler.h"
#include "Events/EventFuncFwd.h"
#include "Events/KeyEvent.h"
#include "Math/CriticalDampedSpring.h"
#include "Math/MathFunctions.h"
#include "Math/EasingFunctions.h"
#include "System/Hash.h"
#include "TransformSystem/TransformSystem.h"
#include "Util/Algorithm.h"
#include "Util/Random.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <cassert>

namespace
{
    constexpr const uint32_t NO_CALLBACK = std::numeric_limits<uint32_t>::max();
}

namespace tweak_values
{
    constexpr float dead_zone = 1.0f;
    constexpr float halflife = 0.2f;
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
    , m_camera_shake_timer_ms(0)
    , m_animation_components(n)
    , m_restore_components(n)
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

    m_key_down_token = m_event_handler->AddListener(key_down_func);
}

CameraSystem::~CameraSystem()
{
    m_event_handler->RemoveListener(m_key_down_token);
}

uint32_t CameraSystem::Id() const
{
    return hash::Hash(Name());
}

const char* CameraSystem::Name() const
{
    return "camerasystem";
}

void CameraSystem::Update(const mono::UpdateContext& update_context)
{
    if(!m_follow_entities.empty())
    {
        std::vector<math::Vector> points;
        for(uint32_t entity_id : m_follow_entities)
            points.push_back(m_transform_system->GetWorldPosition(entity_id));

        const math::Vector centroid = math::CentroidOfPolygon(points);
        const math::Vector camera_position = m_camera->GetTargetPosition();

        const float distance = math::DistanceBetween(centroid, camera_position);
        if(distance > tweak_values::dead_zone)
        {
            math::Vector local_camera_position = camera_position;
            math::critical_spring_damper(
                local_camera_position,
                m_current_camera_speed,
                centroid,
                math::ZeroVec,
                tweak_values::halflife,
                update_context.delta_s);

            m_camera->SetTargetPosition(local_camera_position);
        }

        if(game::g_draw_camera_debug)
        {
            g_debug_drawer->DrawPoint(centroid, 4.0f, mono::Color::MAGENTA);
            g_debug_drawer->DrawCircle(camera_position, tweak_values::dead_zone, mono::Color::MAGENTA);
        }
    }

    const auto process_camera_anims = [this](CameraAnimationComponent* camera_anim)
    {
        PushCameraData(camera_anim->entity_id);

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
            FollowEntity(camera_anim->entity_id);
            break;
        };

        return true;
    };

    mono::remove_if(m_camera_anims_to_process, process_camera_anims);

    if(m_camera_shake_timer_ms > 0)
    {
        constexpr float magnitude = 0.1f;

        math::Vector camera_shake(
            mono::Random(-magnitude, magnitude),
            mono::Random(-magnitude, magnitude)
        );

        m_camera_shake_timer_ms -= update_context.delta_ms;
        m_camera_shake_timer_ms = std::max(m_camera_shake_timer_ms, 0);

        if(m_camera_shake_timer_ms == 0)
            camera_shake = math::ZeroVec;

        m_camera->SetPositionOffset(camera_shake);
    }
}

void CameraSystem::Reset()
{
    m_follow_entities.clear();

    //while(!m_camera_stack.empty())
    //    m_camera_stack.pop();
}

void CameraSystem::FollowEntity(uint32_t entity_id)
{
    m_follow_entities.push_back(entity_id);
}

void CameraSystem::FollowEntityOffset(const math::Vector& offset)
{
    m_current_follow_offset = offset;
}

void CameraSystem::Unfollow(uint32_t entity_id)
{
    mono::remove(m_follow_entities, entity_id);
}

void CameraSystem::PushCameraData(uint32_t entity_id)
{
    CameraStackData stack_data;
    stack_data.camera_size = m_current_camera_size = m_camera->GetTargetViewportSize();
    stack_data.follow_entities = m_follow_entities;
    stack_data.follow_offset = m_current_follow_offset;
    stack_data.debug_entity_id = entity_id;

    m_camera_stack.push(stack_data);
}

void CameraSystem::PopCameraData()
{
    assert(!m_camera_stack.empty());

    const CameraStackData stack_data = m_camera_stack.top();
    m_current_camera_size = stack_data.camera_size;
    m_follow_entities = stack_data.follow_entities;
    m_current_follow_offset = stack_data.follow_offset;

    m_camera->SetTargetViewportSize(m_current_camera_size);

    m_camera_stack.pop();
}

CameraAnimationComponent* CameraSystem::AllocateCameraAnimation(uint32_t entity_id)
{
    CameraAnimationComponent* component = m_animation_components.Set(entity_id, CameraAnimationComponent());
    std::memset(component, 0, sizeof(CameraAnimationComponent));
    component->callback_id = NO_CALLBACK;

    return component;
}

void CameraSystem::ReleaseCameraAnimation(uint32_t entity_id)
{
    CameraAnimationComponent* component = m_animation_components.Get(entity_id);
    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    m_animation_components.Release(entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, float new_zoom_level)
{
    CameraAnimationComponent* component = m_animation_components.Get(entity_id);
    component->trigger_hash = trigger_hash;
    component->type = CameraAnimationType::ZOOM_LEVEL;
    component->zoom_level = new_zoom_level;

    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    const TriggerCallback callback = [this, component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(component);
    };

    component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, const math::Vector& world_point)
{
    CameraAnimationComponent* component = m_animation_components.Get(entity_id);
    component->trigger_hash = trigger_hash;
    component->type = CameraAnimationType::CENTER_ON_POINT;
    component->point_x = world_point.x;
    component->point_y = world_point.y;

    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    const TriggerCallback callback = [this, component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(component);
    };

    component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash, uint32_t follow_entity_id)
{
    CameraAnimationComponent* component = m_animation_components.Get(entity_id);
    component->trigger_hash = trigger_hash;
    component->type = CameraAnimationType::CENTER_ON_ENTITY;
    component->entity_id = follow_entity_id;

    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    const TriggerCallback callback = [this, component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(component);
    };

    component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

CameraRestoreComponent* CameraSystem::AllocateRestoreComponent(uint32_t entity_id)
{
    CameraRestoreComponent* component = m_restore_components.Set(entity_id, CameraRestoreComponent());
    component->callback_id = NO_CALLBACK;
    return component;
}

void CameraSystem::ReleaseRestoreComponent(uint32_t entity_id)
{
    m_restore_components.Release(entity_id);
}

void CameraSystem::AddRestoreComponent(uint32_t entity_id, uint32_t trigger_hash)
{
    CameraRestoreComponent* component = m_restore_components.Get(entity_id);
    component->trigger_hash = trigger_hash;

    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    
    const auto callback = [this](uint32_t trigger_id) {
        PopCameraData();
    };

    component->callback_id = m_trigger_system->RegisterTriggerCallback(component->trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraShake(uint32_t time_ms)
{
    m_camera_shake_timer_ms = std::max(m_camera_shake_timer_ms, (int)time_ms);
}
