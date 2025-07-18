
#include "ReactivePropLogic.h"

#include "EntitySystem/IEntityManager.h"
#include "Physics/PhysicsSystem.h"
#include "SystemContext.h"

using namespace game;

namespace tweak_values
{
    //constexpr float decoy_time_s = 10.0f;
    //constexpr int pushed_times_until_destroyed = 3;
}

ReactivePropLogic::ReactivePropLogic(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_entity_id(entity_id)
//    , m_entity_manager(entity_manager)
    , m_alive_timer_s(0.0f)
    , m_pushed_times(0)
{
    m_entity_manager = system_context->GetSystem<mono::IEntityManager>();
    m_physics_system = system_context->GetSystem<mono::PhysicsSystem>();

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    body->AddCollisionHandler(this);
}

void ReactivePropLogic::Update(const mono::UpdateContext& update_context)
{
    m_alive_timer_s += update_context.delta_s;

    mono::IBody* body = m_physics_system->GetBody(m_entity_id);
    const bool is_sleeping = body->IsSleeping();
    if(is_sleeping)
    {
        m_ready_for_push = true;
    }

    //if(m_pushed_times >= tweak_values::pushed_times_until_destroyed)
    //    m_entity_manager->ReleaseEntity(m_entity_id);
}

mono::CollisionResolve ReactivePropLogic::OnCollideWith(
    mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories)
{
    if(m_ready_for_push)
    {
        mono::IBody* my_body = m_physics_system->GetBody(m_entity_id);
        my_body->ApplyLocalImpulse(collision_normal * 40.0f, math::ZeroVec);
        my_body->ApplyLocalImpulse(collision_normal * 1.0f, math::Vector(0.0f, 0.1f));
        m_ready_for_push = false;

        m_pushed_times++;
    }

    return mono::CollisionResolve::IGNORE;
}

void ReactivePropLogic::OnSeparateFrom(mono::IBody* body)
{
   
}

