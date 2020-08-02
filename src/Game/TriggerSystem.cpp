
#include "TriggerSystem.h"
#include "DamageSystem.h"
#include "Util/Hash.h"
#include "Physics/IShape.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Color.h"

#include "Factories.h"
#include "IDebugDrawer.h"

#include <cassert>

using namespace game;

namespace
{
    class TriggerHandler : public mono::ICollisionHandler
    {
    public:

        TriggerHandler(uint32_t trigger_hash, TriggerSystem* trigger_system)
            : m_trigger_hash(trigger_hash)
            , m_trigger_system(trigger_system)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, uint32_t categories) override
        {
            m_trigger_system->EmitTrigger(m_trigger_hash, TriggerState::ENTER);
            return mono::CollisionResolve::IGNORE;
        }

        void OnSeparateFrom(mono::IBody* body) override
        {
            m_trigger_system->EmitTrigger(m_trigger_hash, TriggerState::EXIT);
        }

        uint32_t m_trigger_hash;
        TriggerSystem* m_trigger_system;
    };

    constexpr uint32_t NO_CALLBACK_SET = std::numeric_limits<uint32_t>::max();
}

TriggerSystem::TriggerSystem(size_t n_triggers, DamageSystem* damage_system, mono::PhysicsSystem* physics_system)
    : m_triggers(n_triggers)
    , m_active(n_triggers, false)
    , m_damage_system(damage_system)
    , m_physics_system(physics_system)
{ }

TriggerComponent* TriggerSystem::AllocateTrigger(uint32_t entity_id)
{
    m_active[entity_id] = true;

    TriggerComponent* allocated_trigger = &m_triggers[entity_id];
    allocated_trigger->death_trigger_id = NO_CALLBACK_SET;
    allocated_trigger->shape_trigger_handler = nullptr;

    return allocated_trigger;
}

void TriggerSystem::ReleaseTrigger(uint32_t entity_id)
{
    TriggerComponent& allocated_trigger = m_triggers[entity_id];
    if(allocated_trigger.death_trigger_id != NO_CALLBACK_SET)
    {
        m_damage_system->RemoveCallback(entity_id, allocated_trigger.death_trigger_id);
        allocated_trigger.death_trigger_id = NO_CALLBACK_SET;
    }

    if(allocated_trigger.shape_trigger_handler)
    {
        mono::IBody* body = m_physics_system->GetBody(entity_id);
        if(body)
            body->RemoveCollisionHandler(allocated_trigger.shape_trigger_handler.get());

        allocated_trigger.shape_trigger_handler = nullptr;
    }

    m_active[entity_id] = false;
}

void TriggerSystem::AddShapeTrigger(uint32_t entity_id, uint32_t trigger_hash, uint32_t collision_mask)
{
    TriggerComponent& allocated_trigger = m_triggers[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        if(allocated_trigger.shape_trigger_handler)
            body->RemoveCollisionHandler(allocated_trigger.shape_trigger_handler.get());

        allocated_trigger.shape_trigger_handler = std::make_unique<TriggerHandler>(trigger_hash, this);
        body->AddCollisionHandler(allocated_trigger.shape_trigger_handler.get());

        std::vector<mono::IShape*> shapes = m_physics_system->GetShapesAttachedToBody(entity_id);
        for(mono::IShape* shape : shapes)
        {
            if(shape->IsSensor())
                shape->SetCollisionMask(collision_mask);
        }
    }
}

void TriggerSystem::AddDeathTrigger(uint32_t entity_id, uint32_t trigger_hash)
{
    TriggerComponent& allocated_trigger = m_triggers[entity_id];

    if(allocated_trigger.death_trigger_id != NO_CALLBACK_SET)
        m_damage_system->RemoveCallback(entity_id, allocated_trigger.death_trigger_id);

    const DestroyedCallback callback = [this, trigger_hash](uint32_t id) {
        EmitTrigger(trigger_hash, TriggerState::ENTER);
    };

    allocated_trigger.death_trigger_id = m_damage_system->SetDestroyedCallback(entity_id, callback);
}

uint32_t TriggerSystem::RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback)
{
    TriggerCallbacks& callback_array = m_trigger_callbacks[trigger_hash];
    const auto it = std::find(callback_array.begin(), callback_array.end(), nullptr);
    if(it != callback_array.end())
    {
        (*it) = callback;
        return std::distance(callback_array.begin(), it);
    }

    return std::numeric_limits<uint32_t>::max();
}

void TriggerSystem::RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id)
{
    m_trigger_callbacks[trigger_hash][callback_id] = nullptr;
}

void TriggerSystem::EmitTrigger(uint32_t trigger_hash, TriggerState state)
{
    m_triggers_to_emit.push_back({ trigger_hash, state });
}

uint32_t TriggerSystem::Id() const
{
    return mono::Hash(Name());
}

const char* TriggerSystem::Name() const
{
    return "TriggerSystem";
}

uint32_t TriggerSystem::Capacity() const
{
    return m_triggers.capacity();
}

void TriggerSystem::Update(const mono::UpdateContext& update_context)
{
    for(const EmitData& emit_data : m_triggers_to_emit)
    {
        const auto it = m_trigger_callbacks.find(emit_data.hash);
        if(it != m_trigger_callbacks.end())
        {
            for(TriggerCallback callback : it->second)
            {
                if(callback)
                    callback(emit_data.hash, emit_data.state);
            }
        }

        const bool enter = (emit_data.state == TriggerState::ENTER);
        const char* suffix = enter ? "Enter" : "Exit";
        game::g_debug_drawer->DrawScreenText(suffix, math::Vector(1, 1), mono::Color::BLACK);
    }

    m_triggers_to_emit.clear();
}
