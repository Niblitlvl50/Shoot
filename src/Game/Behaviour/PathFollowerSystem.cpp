
#include "PathFollowerSystem.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "TransformSystem/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Paths/PathSystem.h"
#include "Paths/PathFactory.h"
#include "Paths/IPath.h"
#include "Math/Matrix.h"
#include "Debug/GameDebugVariables.h"
#include "Debug/IDebugDrawer.h"
#include "Rendering/Color.h"

#include <algorithm>

using namespace game;

PathFollowerSystem::PathFollowerSystem(mono::SystemContext* system_context)
    : m_system_context(system_context)
{ }

PathFollowerComponent* PathFollowerSystem::AllocatePathFollower(uint32_t entity_id)
{
    return &m_components[entity_id];
}

void PathFollowerSystem::ReleasePathFollower(uint32_t entity_id)
{
    m_components.erase(entity_id);
}

void PathFollowerSystem::SetPathFollowerData(
    uint32_t entity_id,
    uint32_t path_entity_reference,
    float speed,
    bool loop,
    bool ping_pong,
    bool apply_rotation,
    const math::Vector& offset,
    bool manual_control)
{
    const auto it = m_components.find(entity_id);
    if(it == m_components.end())
        return;

    PathFollowerComponent* component = &it->second;

    mono::PhysicsSystem* physics_system = m_system_context->GetSystem<mono::PhysicsSystem>();
    mono::IBody* body = physics_system->GetBody(entity_id);

    component->behaviour.Init(body);
    component->behaviour.SetTrackingSpeed(speed);
    component->behaviour.SetLoop(loop);
    component->behaviour.SetPingPong(ping_pong);
    component->behaviour.SetApplyRotation(apply_rotation);
    component->behaviour.SetOffset(offset);
    component->behaviour.SetManualControl(manual_control);

    SetPathReference(entity_id, path_entity_reference, 0.0f);
}

void PathFollowerSystem::SetPathReference(uint32_t entity_id, uint32_t path_entity_reference, float initial_position)
{
    const auto it = m_components.find(entity_id);
    if(it == m_components.end())
        return;

    PathFollowerComponent* component = &it->second;

    // The referenced path entity may not have its own components set up yet (entity
    // creation order isn't guaranteed), so resolve and bake the path in Sync() instead.
    component->pending_path_entity_reference = path_entity_reference;
    component->needs_path_resolve = true;
    component->initial_position = initial_position;
}

void PathFollowerSystem::SetPaused(uint32_t entity_id, bool paused)
{
    const auto it = m_components.find(entity_id);
    if(it != m_components.end())
        it->second.behaviour.SetPaused(paused);
}

void PathFollowerSystem::SetSpeed(uint32_t entity_id, float speed)
{
    const auto it = m_components.find(entity_id);
    if(it != m_components.end())
        it->second.behaviour.SetTrackingSpeed(speed);
}

void PathFollowerSystem::SetOffset(uint32_t entity_id, const math::Vector& offset)
{
    const auto it = m_components.find(entity_id);
    if(it != m_components.end())
        it->second.behaviour.SetOffset(offset);
}

void PathFollowerSystem::SetCurrentPosition(uint32_t entity_id, float position)
{
    const auto it = m_components.find(entity_id);
    if(it != m_components.end())
        it->second.behaviour.SetCurrentPosition(position);
}

void PathFollowerSystem::SetThrottle(uint32_t entity_id, float throttle)
{
    const auto it = m_components.find(entity_id);
    if(it != m_components.end())
        it->second.behaviour.SetThrottle(throttle);
}

float PathFollowerSystem::GetThrottle(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    return (it != m_components.end()) ? it->second.behaviour.GetThrottle() : 0.0f;
}

uint32_t PathFollowerSystem::GetCurrentPathEntity(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    return (it != m_components.end()) ? it->second.current_path_entity_id : mono::INVALID_ID;
}

bool PathFollowerSystem::IsAtPathStart(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    if(it == m_components.end())
        return false;

    constexpr float epsilon = 0.01f;
    return it->second.behaviour.GetCurrentPosition() <= epsilon;
}

bool PathFollowerSystem::IsAtPathEnd(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    if(it == m_components.end())
        return false;

    constexpr float epsilon = 0.01f;
    return it->second.behaviour.GetCurrentPosition() >= (it->second.behaviour.GetPathLength() - epsilon);
}

const std::vector<math::Vector>* PathFollowerSystem::GetPathPoints(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    return (it != m_components.end()) ? it->second.behaviour.GetPathPoints() : nullptr;
}

float PathFollowerSystem::GetCurvature(uint32_t entity_id) const
{
    const auto it = m_components.find(entity_id);
    return (it != m_components.end()) ? it->second.behaviour.GetCurvature() : 0.0f;
}

bool PathFollowerSystem::SwitchToPathEntity(
    uint32_t entity_id, uint32_t new_path_entity_id, const math::Vector& enter_at_world_position, bool entering_forward)
{
    const auto it = m_components.find(entity_id);
    if(it == m_components.end())
        return false;

    mono::IPathPtr new_path = BakePath(new_path_entity_id);
    if(!new_path)
        return false;

    std::vector<math::Vector> points = new_path->GetPathPoints();
    if(points.empty())
        return false;

    constexpr float snap_tolerance = 0.5f;
    const bool entry_matches_front = math::DistanceBetween(points.front(), enter_at_world_position) <= snap_tolerance;
    const bool entry_matches_back = math::DistanceBetween(points.back(), enter_at_world_position) <= snap_tolerance;
    if(!entry_matches_front && !entry_matches_back)
        return false;

    // Forward entries need the entry point at the start (position 0); backward entries need
    // it at the far end (position Length()) - either way, continuing with the same throttle
    // direction moves away from the entry point instead of clamping right back against it.
    const bool need_reverse = entering_forward ? entry_matches_back : entry_matches_front;
    if(need_reverse)
        std::reverse(points.begin(), points.end());

    mono::IPathPtr oriented_path = mono::CreatePath(points);
    const float entry_position = entering_forward ? 0.0f : oriented_path->Length();

    it->second.behaviour.SetPath(std::move(oriented_path));
    it->second.behaviour.SetCurrentPosition(entry_position);
    it->second.current_path_entity_id = new_path_entity_id;
    return true;
}

mono::IPathPtr PathFollowerSystem::BakePath(uint32_t path_entity_id) const
{
    mono::PathSystem* path_system = m_system_context->GetSystem<mono::PathSystem>();
    const mono::PathComponent* path_component = path_system->GetPath(path_entity_id);
    if(!path_component)
        return nullptr;

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    const math::Matrix& path_transform = transform_system->GetWorld(path_entity_id);

    std::vector<math::Vector> world_points;
    world_points.reserve(path_component->points.size());
    for(const math::Vector& local_point : path_component->points)
        world_points.push_back(math::Transformed(path_transform, local_point));

    return mono::CreatePath(world_points, path_component->type);
}

const char* PathFollowerSystem::Name() const
{
    return "pathfollowersystem";
}

void PathFollowerSystem::Sync()
{
    if(m_components.empty())
        return;

    mono::IEntityManager* entity_manager = m_system_context->GetSystem<mono::IEntityManager>();

    for(auto& entity_component_pair : m_components)
    {
        PathFollowerComponent& component = entity_component_pair.second;
        if(!component.needs_path_resolve)
            continue;

        const uint32_t path_entity_id = entity_manager->GetEntityIdFromUuid(component.pending_path_entity_reference);
        if(path_entity_id == mono::INVALID_ID)
            continue;

        mono::IPathPtr path = BakePath(path_entity_id);
        if(!path)
            continue;

        component.current_path_entity_id = path_entity_id;
        component.behaviour.SetPath(std::move(path));
        component.behaviour.TeleportToPosition(component.initial_position);
        component.needs_path_resolve = false;

        m_system_context->GetSystem<mono::TransformSystem>()->SetTransformState(entity_component_pair.first, mono::TransformState::PHYSICS);
    }
}

void PathFollowerSystem::Update(const mono::UpdateContext& update_context)
{
    for(auto& entity_component_pair : m_components)
        entity_component_pair.second.behaviour.Run(update_context.delta_s);

    if(!game::g_draw_path_follower_debug)
        return;

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();

    for(const auto& entity_component_pair : m_components)
    {
        const PathDebugData debug_data = entity_component_pair.second.behaviour.GetDebugData();
        if(!debug_data.has_path)
            continue;

        const math::Vector world_position = transform_system->GetWorldPosition(entity_component_pair.first);

        game::g_debug_drawer->DrawLine(*debug_data.path_points, 1.0f, mono::Color::BLUE);
        game::g_debug_drawer->DrawPoint(debug_data.target_position, 8.0f, mono::Color::GREEN);
        game::g_debug_drawer->DrawLine(world_position, debug_data.target_position, 1.0f, mono::Color::GREEN);
    }
}
