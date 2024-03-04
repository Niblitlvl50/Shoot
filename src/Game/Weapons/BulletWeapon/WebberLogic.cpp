
#include "WebberLogic.h"

#include "EntitySystem/IEntityManager.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/IBody.h"
#include "Physics/IConstraint.h"
#include "Rendering/IRenderer.h"
#include "Rendering/Color.h"
#include "TransformSystem/TransformSystem.h"

using namespace game;

WebberLogic::WebberLogic(uint32_t entity_id, mono::IEntityManager* entity_system, mono::PhysicsSystem* physics_system, mono::TransformSystem* transform_system)
    : m_entity_id(entity_id)
    , m_entity_system(entity_system)
    , m_physics_system(physics_system)
    , m_transform_system(transform_system)
    , m_life_timer(0.0f)
{ }

void WebberLogic::Destroy()
{
    for(mono::IConstraint* constraint : m_constraints)
        m_physics_system->ReleaseConstraint(constraint);

    m_constraints.clear();

    m_entity_system->ReleaseEntity(m_entity_id);
}

void WebberLogic::Update(const mono::UpdateContext& update_context)
{
   m_life_timer += update_context.delta_s;

   if(m_life_timer > 5.0f)
   {
        // release me
        Destroy();
   }
}

void WebberLogic::Draw(mono::IRenderer& renderer) const
{
    const math::Vector& world_position = m_transform_system->GetWorldPosition(m_entity_id);

    std::vector<math::Vector> lines;
    
    for(const AttachedBody& body_pair : m_attached_bodies)
    {
        const math::Vector& body_world_position = body_pair.body->LocalToWorld(body_pair.local_offset);
        lines.push_back(world_position);
        lines.push_back(body_world_position);
    }

    renderer.DrawLines(lines, mono::Color::RED, 1.0f);
}

void WebberLogic::AttachTo(mono::IBody* body, const math::Vector& world_position)
{
    const AttachedBody body_pair = { body, body->WorldToLocal(world_position) };
    m_attached_bodies.push_back(body_pair);

    const math::Vector& entity_world_position = m_transform_system->GetWorldPosition(m_entity_id);
    const float distance_between_bodies = math::DistanceBetween(entity_world_position, world_position);

    mono::IBody* entity_body = m_physics_system->GetBody(m_entity_id);
    mono::IConstraint* constraint =
        m_physics_system->CreateSpring(entity_body, body, math::ZeroVec, body_pair.local_offset, distance_between_bodies, 10.0f, 0.1f);

    m_constraints.push_back(constraint);
}
