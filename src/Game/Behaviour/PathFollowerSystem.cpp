
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
    const math::Vector& offset)
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

    mono::IEntityManager* entity_manager = m_system_context->GetSystem<mono::IEntityManager>();
    const uint32_t path_entity_id = entity_manager->GetEntityIdFromUuid(path_entity_reference);
    if(path_entity_id == mono::INVALID_ID)
        return;

    mono::PathSystem* path_system = m_system_context->GetSystem<mono::PathSystem>();
    const mono::PathComponent* path_component = path_system->GetPath(path_entity_id);
    if(!path_component)
        return;

    mono::TransformSystem* transform_system = m_system_context->GetSystem<mono::TransformSystem>();
    const math::Matrix& path_transform = transform_system->GetWorld(path_entity_id);

    std::vector<math::Vector> world_points;
    world_points.reserve(path_component->points.size());
    for(const math::Vector& local_point : path_component->points)
        world_points.push_back(math::Transformed(path_transform, local_point));

    component->behaviour.SetPath(mono::CreatePath(world_points, path_component->type));
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

const char* PathFollowerSystem::Name() const
{
    return "pathfollowersystem";
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
