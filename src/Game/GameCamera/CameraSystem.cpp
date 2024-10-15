
#include "CameraSystem.h"
#include "Player/PlayerInfo.h"
#include "TriggerSystem/TriggerSystem.h"

// Debug
#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"
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
    //constexpr float halflife = 0.2f;
    constexpr float screen_shake_magnitude = 0.05f;
}

using namespace game;

CameraSystem::CameraSystem(
    uint32_t n, mono::ICamera* camera, mono::TransformSystem* transform_system, mono::EventHandler* event_handler, mono::TriggerSystem* trigger_system)
    : m_camera(camera)
    , m_transform_system(transform_system)
    , m_trigger_system(trigger_system)
    , m_controller(camera, event_handler)
    , m_camera_shake_timer_s(0.0f)
    , m_animation_components(n)
    , m_restore_components(n)
{ }

const char* CameraSystem::Name() const
{
    return "camerasystem";
}

void CameraSystem::Update(const mono::UpdateContext& update_context)
{
    if(!m_follow_entities.empty())
    {
        math::Vector centroid;

        if(m_follow_entities.size() == 1)
        {
            centroid = m_transform_system->GetWorldPosition(m_follow_entities.front());
        }
        else
        {
            std::vector<math::Vector> points;
            for(uint32_t entity_id : m_follow_entities)
                points.push_back(m_transform_system->GetWorldPosition(entity_id));

            const math::Vector world_centroid = math::CentroidOfPolygon(points);

            for(uint32_t index = 0; index < points.size(); ++index)
            {
                const uint32_t entity_id = m_follow_entities[index];
                math::Vector& world_position = points[index];

                const float weight = game::IsPlayer(entity_id) ? 1.0f : 0.5f;
                world_position = (world_position - world_centroid) * weight;
            }

            centroid = math::CentroidOfPolygon(points) + world_centroid;
        }

        const math::Vector camera_position = m_camera->GetTargetPosition();
        const math::Vector& delta = camera_position - centroid;
        const float delta_length = math::Length(delta);

        const math::Vector target_point = (delta_length < tweak_values::dead_zone) ?
            camera_position : (centroid + math::Normalized(delta) * tweak_values::dead_zone);

        m_camera->SetTargetPosition(target_point);

        if(game::g_draw_camera_debug)
        {
            g_debug_drawer->DrawLine({ m_camera->GetPosition(), target_point }, 1.0f, mono::Color::BLUE);
            g_debug_drawer->DrawPoint(centroid, 4.0f, mono::Color::MAGENTA);
            g_debug_drawer->DrawPoint(target_point, 4.0f, mono::Color::BLUE);
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
        {
            const math::Vector world_position = m_transform_system->GetWorldPosition(camera_anim->entity_id);
            m_camera->SetTargetPosition(world_position);
            break;
        }
        case CameraAnimationType::CENTER_ON_ENTITY:
            FollowEntity(camera_anim->entity_id);
            break;
        };

        return true;
    };

    mono::remove_if(m_camera_anims_to_process, process_camera_anims);

    if(m_camera_shake_timer_s > 0.0f)
    {
        m_camera_shake_timer_s -= update_context.delta_s;
        m_camera_shake_timer_s = std::max(m_camera_shake_timer_s, 0.0f);

        constexpr float magnitude = tweak_values::screen_shake_magnitude;
        const math::Vector camera_shake = (m_camera_shake_timer_s == 0.0f) ? 
            math::ZeroVec : math::Vector(mono::Random(-magnitude, magnitude), mono::Random(-magnitude, magnitude));

        m_camera->SetPositionOffset(camera_shake);
    }

    // Enable/Disable the debug camera controller, not great to do it here in Update.
    game::g_debug_camera ? m_controller.Enable() : m_controller.Disable();
}

void CameraSystem::Reset()
{
    m_follow_entities.clear();
    m_camera_anims_to_process.clear();

    while(!m_camera_stack.empty())
        PopCameraData();
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
    MONO_ASSERT(!m_camera_stack.empty());

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

    const mono::TriggerCallback callback = [this, component](uint32_t trigger_id) {
        m_camera_anims_to_process.push_back(component);
    };

    component->callback_id = m_trigger_system->RegisterTriggerCallback(trigger_hash, callback, entity_id);
}

void CameraSystem::AddCameraAnimationComponent(uint32_t entity_id, uint32_t trigger_hash)
{
    CameraAnimationComponent* component = m_animation_components.Get(entity_id);
    component->trigger_hash = trigger_hash;
    component->type = CameraAnimationType::CENTER_ON_POINT;
    component->entity_id = entity_id;

    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

    const mono::TriggerCallback callback = [this, component](uint32_t trigger_id) {
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

    const mono::TriggerCallback callback = [this, component](uint32_t trigger_id) {
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
    CameraRestoreComponent* component = m_restore_components.Get(entity_id);
    if(component->callback_id != NO_CALLBACK)
        m_trigger_system->RemoveTriggerCallback(component->trigger_hash, component->callback_id, entity_id);

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

void CameraSystem::AddCameraShake(float time_s)
{
    m_camera_shake_timer_s = std::max(m_camera_shake_timer_s, time_s);
}

const mono::ICamera* CameraSystem::GetActiveCamera() const
{
    return m_camera;
}

mono::ICamera* CameraSystem::GetActiveCamera()
{
    return m_camera;
}
