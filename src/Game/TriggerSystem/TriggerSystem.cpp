
#include "TriggerSystem.h"
#include "DamageSystem.h"
#include "ConditionSystem/ConditionSystem.h"
#include "Factories.h"
#include "IDebugDrawer.h"
#include "GameDebug.h"

#include "Util/Algorithm.h"
#include "System/Hash.h"
#include "Physics/IShape.h"
#include "Physics/IBody.h"
#include "Physics/PhysicsSystem.h"
#include "Physics/PhysicsSpace.h"
#include "Rendering/Color.h"
#include "EntitySystem/IEntityManager.h"

#include <cassert>
#include <string>

using namespace game;

namespace
{
    class TriggerHandler : public mono::ICollisionHandler
    {
    public:

        TriggerHandler(uint32_t trigger_hash_enter, uint32_t trigger_hash_exit, TriggerSystem* trigger_system)
            : m_trigger_hash_enter(trigger_hash_enter)
            , m_trigger_hash_exit(trigger_hash_exit)
            , m_trigger_system(trigger_system)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, const math::Vector& collision_normal, uint32_t categories) override
        {
            m_trigger_system->EmitTrigger(m_trigger_hash_enter);
            return mono::CollisionResolve::IGNORE;
        }

        void OnSeparateFrom(mono::IBody* body) override
        {
            m_trigger_system->EmitTrigger(m_trigger_hash_exit);
        }

        const uint32_t m_trigger_hash_enter;
        const uint32_t m_trigger_hash_exit;
        TriggerSystem* m_trigger_system;
    };

    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

TriggerSystem::TriggerSystem(
    size_t n_triggers, DamageSystem* damage_system, mono::PhysicsSystem* physics_system, mono::IEntityManager* entity_system)
    : m_damage_system(damage_system)
    , m_physics_system(physics_system)
    , m_entity_system(entity_system)
    , m_shape_triggers(n_triggers)
    , m_destroyed_triggers(n_triggers)
    , m_area_triggers(n_triggers)
    , m_time_triggers(n_triggers)
    , m_counter_triggers(n_triggers)
    , m_relay_triggers(n_triggers)
    , m_area_trigger_timer(0)
{ }

ShapeTriggerComponent* TriggerSystem::AllocateShapeTrigger(uint32_t entity_id)
{
    ShapeTriggerComponent* allocated_trigger = m_shape_triggers.Set(entity_id, game::ShapeTriggerComponent());
    allocated_trigger->shape_trigger_handler = nullptr;
    return allocated_trigger;
}

void TriggerSystem::ReleaseShapeTrigger(uint32_t entity_id)
{
    ShapeTriggerComponent* allocated_trigger = m_shape_triggers.Get(entity_id);

    if(allocated_trigger->shape_trigger_handler)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->RemoveCollisionHandler(allocated_trigger->shape_trigger_handler.get());

        allocated_trigger->shape_trigger_handler = nullptr;
    }

    m_shape_triggers.Release(entity_id);
}

void TriggerSystem::AddShapeTrigger(
    uint32_t entity_id, uint32_t trigger_hash_enter, uint32_t trigger_hash_exit, uint32_t collision_mask)
{
    ShapeTriggerComponent* allocated_trigger = m_shape_triggers.Get(entity_id);
    allocated_trigger->trigger_hash_enter = trigger_hash_enter;
    allocated_trigger->trigger_hash_exit = trigger_hash_exit;

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        if(allocated_trigger->shape_trigger_handler)
            body->RemoveCollisionHandler(allocated_trigger->shape_trigger_handler.get());

        allocated_trigger->shape_trigger_handler = std::make_unique<TriggerHandler>(trigger_hash_enter, trigger_hash_exit, this);
        body->AddCollisionHandler(allocated_trigger->shape_trigger_handler.get());

        std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(entity_id);
        for(mono::IShape* shape : shapes)
        {
            if(shape->IsSensor())
                shape->SetCollisionMask(collision_mask);
        }
    }
}

DestroyedTriggerComponent* TriggerSystem::AllocateDestroyedTrigger(uint32_t entity_id)
{
    DestroyedTriggerComponent* allocated_trigger = m_destroyed_triggers.Set(entity_id, game::DestroyedTriggerComponent());
    allocated_trigger->callback_id = NO_CALLBACK_SET;
    return allocated_trigger;
}

void TriggerSystem::ReleaseDestroyedTrigger(uint32_t entity_id)
{
    DestroyedTriggerComponent* allocated_trigger = m_destroyed_triggers.Get(entity_id);

    switch(allocated_trigger->trigger_type)
    {
    case shared::DestroyedTriggerType::ON_DEATH:
        m_damage_system->RemoveDamageCallback(entity_id, allocated_trigger->callback_id);
        break;
    case shared::DestroyedTriggerType::ON_DESTORYED:
        m_entity_system->RemoveReleaseCallback(entity_id, allocated_trigger->callback_id);
        break;
    };
    allocated_trigger->callback_id = NO_CALLBACK_SET;

    m_destroyed_triggers.Release(entity_id);
}

void TriggerSystem::AddDestroyedTrigger(uint32_t entity_id, uint32_t trigger_hash, shared::DestroyedTriggerType type)
{
    DestroyedTriggerComponent* allocated_trigger = m_destroyed_triggers.Get(entity_id);
    allocated_trigger->trigger_hash = trigger_hash;

    switch(type)
    {
    case shared::DestroyedTriggerType::ON_DEATH:
    {
        if(allocated_trigger->callback_id != NO_CALLBACK_SET)
            m_damage_system->RemoveDamageCallback(entity_id, allocated_trigger->callback_id);

        const DamageCallback callback = [this, trigger_hash](uint32_t id, int damage, uint32_t id_who_did_damage, DamageType type) {
            EmitTrigger(trigger_hash);
        };
        allocated_trigger->callback_id = m_damage_system->SetDamageCallback(entity_id, DamageType::DESTROYED, callback);

        break;
    }
    case shared::DestroyedTriggerType::ON_DESTORYED:
    {
        if(allocated_trigger->callback_id != NO_CALLBACK_SET)
            m_entity_system->RemoveReleaseCallback(entity_id, allocated_trigger->callback_id);

        const mono::ReleaseCallback callback = [this, trigger_hash](uint32_t id) {
            EmitTrigger(trigger_hash);
        };
        allocated_trigger->callback_id = m_entity_system->AddReleaseCallback(entity_id, callback);

        break;
    }
    };
}

AreaEntityTriggerComponent* TriggerSystem::AllocateAreaTrigger(uint32_t entity_id)
{
    AreaEntityTriggerComponent* allocated_trigger = m_area_triggers.Set(entity_id, game::AreaEntityTriggerComponent());
    return allocated_trigger;
}

void TriggerSystem::ReleaseAreaTrigger(uint32_t entity_id)
{
    m_area_triggers.Release(entity_id);
}

void TriggerSystem::AddAreaEntityTrigger(
    uint32_t entity_id, uint32_t trigger_hash, const math::Quad& world_bb, uint32_t faction, shared::AreaTriggerOperation operation, int n_entities)
{
    AreaEntityTriggerComponent* area_trigger = m_area_triggers.Get(entity_id);

    area_trigger->trigger_hash = trigger_hash;
    area_trigger->faction = faction;
    area_trigger->world_bb = world_bb;
    area_trigger->n_entities = n_entities;
    area_trigger->operation = operation;
}

TimeTriggerComponent* TriggerSystem::AllocateTimeTrigger(uint32_t entity_id)
{
    return m_time_triggers.Set(entity_id, game::TimeTriggerComponent());
}

void TriggerSystem::ReleaseTimeTrigger(uint32_t entity_id)
{
    m_time_triggers.Release(entity_id);
}

void TriggerSystem::AddTimeTrigger(uint32_t entity_id, uint32_t trigger_hash, float timeout_ms, bool repeating)
{
    TimeTriggerComponent* time_trigger = m_time_triggers.Get(entity_id);

    time_trigger->trigger_hash = trigger_hash;
    time_trigger->timeout_ms = timeout_ms;
    time_trigger->timeout_counter_ms = timeout_ms;
    time_trigger->repeating = repeating;
}

CounterTriggerComponent* TriggerSystem::AllocateCounterTrigger(uint32_t entity_id)
{
    CounterTriggerComponent* counter_trigger = m_counter_triggers.Set(entity_id, game::CounterTriggerComponent());
    counter_trigger->callback_id = NO_CALLBACK_SET;
    counter_trigger->counter = 0;
    
    return counter_trigger;
}

void TriggerSystem::ReleaseCounterTrigger(uint32_t entity_id)
{
    CounterTriggerComponent* counter_trigger = m_counter_triggers.Get(entity_id);

    if(counter_trigger->callback_id != NO_CALLBACK_SET)
    {
        RemoveTriggerCallback(counter_trigger->listen_trigger_hash, counter_trigger->callback_id, entity_id);
        counter_trigger->callback_id = NO_CALLBACK_SET;
    }

    m_counter_triggers.Release(entity_id);
}

void TriggerSystem::AddCounterTrigger(uint32_t entity_id, uint32_t listener_hash, uint32_t completed_hash, int count, bool reset_on_completed)
{
    CounterTriggerComponent* counter_trigger = m_counter_triggers.Get(entity_id);

    counter_trigger->listen_trigger_hash = listener_hash;
    counter_trigger->completed_trigger_hash = completed_hash;
    counter_trigger->count = count;
    counter_trigger->reset_on_completed = reset_on_completed;

    if(counter_trigger->callback_id != NO_CALLBACK_SET)
    {
        RemoveTriggerCallback(counter_trigger->listen_trigger_hash, counter_trigger->callback_id, entity_id);
        counter_trigger->callback_id = NO_CALLBACK_SET;
    }

    const auto counter_callback = [this, counter_trigger](uint32_t trigger_id)
    {
        counter_trigger->counter++;
        if(counter_trigger->counter == counter_trigger->count)
        {
            EmitTrigger(counter_trigger->completed_trigger_hash);
            if(counter_trigger->reset_on_completed)
                counter_trigger->counter = 0;
            else
            {
                // Remove?
            }
        }
    };

    counter_trigger->callback_id = RegisterTriggerCallback(listener_hash, counter_callback, entity_id);
}

RelayTriggerComponent* TriggerSystem::AllocateRelayTrigger(uint32_t entity_id)
{
    return m_relay_triggers.Set(entity_id, RelayTriggerComponent());
}

void TriggerSystem::ReleaseRelayTrigger(uint32_t entity_id)
{
    RelayTriggerComponent* trigger = m_relay_triggers.Get(entity_id);

    if(trigger->callback_id != NO_CALLBACK_SET)
    {
        RemoveTriggerCallback(trigger->listen_trigger_hash, trigger->callback_id, entity_id);
        trigger->callback_id = NO_CALLBACK_SET;
    }

    m_relay_triggers.Release(entity_id);
}

void TriggerSystem::AddRelayTrigger(uint32_t entity_id, uint32_t listener_hash, uint32_t completed_hash, int delay_ms)
{
    RelayTriggerComponent* trigger = m_relay_triggers.Get(entity_id);
    
    trigger->listen_trigger_hash = listener_hash;
    trigger->completed_trigger_hash = completed_hash;
    trigger->delay_ms = delay_ms;

    if(trigger->callback_id != NO_CALLBACK_SET)
    {
        RemoveTriggerCallback(trigger->listen_trigger_hash, trigger->callback_id, entity_id);
        trigger->callback_id = NO_CALLBACK_SET;
    }

    const auto counter_callback = [this, trigger](uint32_t trigger_id)
    {
        DelayRelayTrigger delay_trigger;
        delay_trigger.relay_trigger_hash = trigger->completed_trigger_hash;
        delay_trigger.delay_ms = trigger->delay_ms;

        m_delay_relay_triggers.push_back(delay_trigger);
    };

    trigger->callback_id = RegisterTriggerCallback(trigger->listen_trigger_hash, counter_callback, entity_id);
}

uint32_t TriggerSystem::RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback, uint32_t debug_entity_id)
{
    m_entity_id_to_trigger_hashes[trigger_hash].push_back(debug_entity_id);

    TriggerCallbacks& callback_array = m_trigger_callbacks[trigger_hash];
    const auto it = std::find(callback_array.begin(), callback_array.end(), nullptr);
    if(it != callback_array.end())
    {
        (*it) = callback;
        return std::distance(callback_array.begin(), it);
    }

    return std::numeric_limits<uint32_t>::max();
}

void TriggerSystem::RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id, uint32_t debug_entity_id)
{
    m_trigger_callbacks[trigger_hash][callback_id] = nullptr;
    mono::remove(m_entity_id_to_trigger_hashes[trigger_hash], debug_entity_id);
}

const std::unordered_map<uint32_t, std::vector<uint32_t>>& TriggerSystem::GetTriggerTargets() const
{
    return m_entity_id_to_trigger_hashes;
}

void TriggerSystem::EmitTrigger(uint32_t trigger_hash)
{
    m_triggers_to_emit.push_back(trigger_hash);
}

uint32_t TriggerSystem::Id() const
{
    return hash::Hash(Name());
}

const char* TriggerSystem::Name() const
{
    return "TriggerSystem";
}

void TriggerSystem::Update(const mono::UpdateContext& update_context)
{
    m_area_trigger_timer += update_context.delta_ms;
    if(m_area_trigger_timer >= 1000)
    {
        UpdateAreaEntityTriggers(update_context);
        m_area_trigger_timer = 0;
    }

    UpdateTimeTriggers(update_context);
    UpdateDelayedRelayTriggers(update_context);

    const std::vector<uint32_t> local_triggers_to_emit = m_triggers_to_emit;
    m_triggers_to_emit.clear();

    for(uint32_t trigger_hash : local_triggers_to_emit)
    {
        const auto it = m_trigger_callbacks.find(trigger_hash);
        if(it != m_trigger_callbacks.end())
        {
            for(TriggerCallback callback : it->second)
            {
                if(callback)
                    callback(trigger_hash);
            }
        }

        if(game::g_draw_triggers)
        {
            const char* hash_string = hash::HashLookup(trigger_hash);
            game::g_debug_drawer->DrawScreenTextFading(hash_string, math::Vector(1, 2), mono::Color::BLACK, 2000);
        }
    }
}

void TriggerSystem::UpdateAreaEntityTriggers(const mono::UpdateContext& update_context)
{
    const auto update_area_entity_trigger = [this](uint32_t index, AreaEntityTriggerComponent& area_trigger) {
        const std::vector<mono::QueryResult> found_bodies
            = m_physics_system->GetSpace()->QueryBox(area_trigger.world_bb, area_trigger.faction);
        const int n_found_bodies = found_bodies.size();

        bool emit_trigger = false;

        switch(area_trigger.operation)
        {
        case shared::AreaTriggerOperation::EQUAL_TO:
            emit_trigger = (n_found_bodies == area_trigger.n_entities);
            break;
        case shared::AreaTriggerOperation::LESS_THAN:
            emit_trigger = (n_found_bodies < area_trigger.n_entities);
            break;
        case shared::AreaTriggerOperation::GREATER_THAN:
            emit_trigger = (n_found_bodies > area_trigger.n_entities);
            break;
        }

        if(emit_trigger)
        {
            EmitTrigger(area_trigger.trigger_hash);
            m_area_triggers.Release(index);
        }
    };

    m_area_triggers.ForEach(update_area_entity_trigger);
}

void TriggerSystem::UpdateTimeTriggers(const mono::UpdateContext& update_context)
{
    const auto update_time_trigger = [this, &update_context](uint32_t index, TimeTriggerComponent& time_trigger) {
        time_trigger.timeout_counter_ms -= update_context.delta_ms;

        const bool emit_trigger = (time_trigger.timeout_counter_ms < 0.0f);
        if(emit_trigger)
        {
            EmitTrigger(time_trigger.trigger_hash);
            if(time_trigger.repeating)
                time_trigger.timeout_counter_ms = time_trigger.timeout_ms;
            else
                m_time_triggers.Release(index);
        }
    };

    m_time_triggers.ForEach(update_time_trigger);
}

void TriggerSystem::UpdateDelayedRelayTriggers(const mono::UpdateContext& update_context)
{
    const auto update_and_remove = [this, &update_context](DelayRelayTrigger& delay_trigger) {
        delay_trigger.delay_ms -= update_context.delta_ms;
        if(delay_trigger.delay_ms <= 0)
        {
            EmitTrigger(delay_trigger.relay_trigger_hash);
            return true;
        }

        return false;
    };
    mono::remove_if(m_delay_relay_triggers, update_and_remove);
}
