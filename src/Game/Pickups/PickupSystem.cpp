
#include "PickupSystem.h"

#include "Util/Hash.h"
#include "Physics/PhysicsSystem.h"

#include "EntitySystem/IEntityManager.h"

using namespace game;

namespace
{
    class PickupHandler : public mono::ICollisionHandler
    {
    public:

        PickupHandler(PickupSystem* pickup_system, uint32_t pickup_id)
            : m_pickup_system(pickup_system)
            , m_pickup_id(pickup_id)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override
        {
            const uint32_t target_id = mono::PhysicsSystem::GetIdFromBody(body);
            m_pickup_system->HandlePickup(m_pickup_id, target_id);

            return mono::CollisionResolve::IGNORE;
        }

        void OnSeparateFrom(mono::IBody* body) override
        { }

        PickupSystem* m_pickup_system;
        uint32_t m_pickup_id;
    };
}

PickupSystem::PickupSystem(uint32_t n, mono::PhysicsSystem* physics_system, mono::IEntityManager* entity_manager)
    : m_physics_system(physics_system)
    , m_entity_manager(entity_manager)
{
    m_pickups.resize(n);
    m_active.resize(n, false);
    m_collision_handlers.resize(n);

    m_pickup_sound = audio::CreateSound("res/sound/item_pickup.wav", audio::SoundPlayback::ONCE);
}

game::Pickup* PickupSystem::AllocatePickup(uint32_t entity_id)
{
    m_active[entity_id] = true;
    game::Pickup* allocated_pickup = &m_pickups[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        auto handler = std::make_unique<PickupHandler>(this, entity_id);
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
    m_pickups[id] = pickup_data;
}

void PickupSystem::HandlePickup(uint32_t pickup_id, uint32_t target_id)
{
    m_pickups_to_process.push_back( { pickup_id, target_id } );
}

void PickupSystem::RegisterPickupTarget(uint32_t target_id, PickupCallback callback)
{
    m_pickup_targets[target_id] = callback;
}

void PickupSystem::UnregisterPickupTarget(uint32_t target_id)
{
    m_pickup_targets.erase(target_id);
}

uint32_t PickupSystem::Id() const
{
    return mono::Hash(Name());
}

const char* PickupSystem::Name() const
{
    return "pickupsystem";
}

void PickupSystem::Update(const mono::UpdateContext& update_context)
{
    for(const PickupToTarget& pickup : m_pickups_to_process)
    {
        const auto it = m_pickup_targets.find(pickup.target_id);
        if(it != m_pickup_targets.end())
        {
            const Pickup& pickup_data = m_pickups[pickup.pickup_id];
            it->second(pickup_data.type, pickup_data.amount);

            PlayPickupSound(pickup_data.type);
        }

        m_entity_manager->ReleaseEntity(pickup.pickup_id);
    }

    m_pickups_to_process.clear();
}

void PickupSystem::PlayPickupSound(shared::PickupType type)
{
    switch(type)
    {
    default:
        m_pickup_sound->Play();
        break;
    };
}
