
#include "PickupSystem.h"
#include "Util/Hash.h"
#include "Physics/IShape.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"

using namespace game;

namespace
{
    class PickupHandler : public mono::ICollisionHandler
    {
    public:

        PickupHandler(const game::Pickup* pickup, game::PickupSystem* pickup_system)
            : m_pickup(pickup)
            , m_pickup_system(pickup_system)
        { }
    
        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, uint32_t categories) override
        {
            m_pickup_system->HandlePickup(mono::PhysicsSystem::GetIdFromBody(body), m_pickup);
            return mono::CollisionResolve::IGNORE;
        }

        void OnSeparateFrom(mono::IBody* body) override
        { }

        const game::Pickup* m_pickup;
        game::PickupSystem* m_pickup_system;
    };
}

PickupSystem::PickupSystem(uint32_t n, mono::PhysicsSystem* physics_system)
    : m_physics_system(physics_system)
{
    m_pickups.resize(n);
    m_active.resize(n, false);
}

game::Pickup* PickupSystem::AllocatePickup(uint32_t entity_id)
{
    m_active[entity_id] = true;
    game::Pickup* allocated_pickup = &m_pickups[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        auto handler = std::make_unique<PickupHandler>(allocated_pickup, this);
        body->AddCollisionHandler(handler.get());
        m_collision_handlers[entity_id] = std::move(handler);
    }

    return allocated_pickup;
}

void PickupSystem::ReleasePickup(uint32_t entity_id)
{
    auto& handler = m_collision_handlers[entity_id];
    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
        body->RemoveCollisionHandler(handler.get());

    m_active[entity_id] = false;
    m_collision_handlers[entity_id] = nullptr;
}

void PickupSystem::SetPickupData(uint32_t id, const Pickup& pickup_data)
{

}

uint32_t PickupSystem::Id() const
{
    return mono::Hash(Name());
}

const char* PickupSystem::Name() const
{
    return "pickupsystem";
}

uint32_t PickupSystem::Capacity() const
{
    return 0;
}

void PickupSystem::Update(const mono::UpdateContext& update_context)
{

}

void PickupSystem::HandlePickup(uint32_t id, const Pickup* pickup)
{

}
