
#include "TriggerSystem.h"
#include "Util/Hash.h"
#include "Physics/IShape.h"
#include "Physics/PhysicsSystem.h"
#include "Rendering/Color.h"

#include "Factories.h"
#include "IDebugDrawer.h"

using namespace game;

namespace
{
    class TriggerHandler : public mono::ICollisionHandler
    {
    public:

        TriggerHandler(TriggerComponent* trigger_component, TriggerSystem* trigger_system)
            : m_trigger_component(trigger_component)
            , m_trigger_system(trigger_system)
        { }

        mono::CollisionResolve OnCollideWith(
            mono::IBody* body, const math::Vector& collision_point, uint32_t categories) override
        {
            m_trigger_system->EmitTrigger(m_trigger_component->trigger_hash);
            return mono::CollisionResolve::IGNORE;
        }

        TriggerComponent* m_trigger_component;
        TriggerSystem* m_trigger_system;
    };
}

TriggerSystem::TriggerSystem(size_t n_triggers, mono::PhysicsSystem* physics_system)
    : m_triggers(n_triggers)
    , m_collision_handlers(n_triggers)
    , m_active(n_triggers, false)
    , m_physics_system(physics_system)
{ }

TriggerComponent* TriggerSystem::AllocateTrigger(uint32_t entity_id)
{
    assert(!m_active[entity_id]);
    m_active[entity_id] = true;
    TriggerComponent* allocated_trigger = &m_triggers[entity_id];

    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
    {
        auto handler = std::make_unique<TriggerHandler>(allocated_trigger, this);
        body->AddCollisionHandler(handler.get());
        m_collision_handlers[entity_id] = std::move(handler);
    }

    return allocated_trigger;
}

void TriggerSystem::ReleaseTrigger(uint32_t entity_id)
{
    auto& handler = m_collision_handlers[entity_id];
    mono::IBody* body = m_physics_system->GetBody(entity_id);
    if(body)
        body->RemoveCollisionHandler(handler.get());

    m_active[entity_id] = false;
    m_collision_handlers[entity_id] = nullptr;
}

void TriggerSystem::SetTriggerData(uint32_t entity_id, const TriggerComponent& component_data)
{
    m_triggers[entity_id] = component_data;
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

void TriggerSystem::EmitTrigger(uint32_t trigger_hash)
{
    m_triggers_to_emit.push_back(trigger_hash);
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
    for(uint32_t hash : m_triggers_to_emit)
    {
        const auto it = m_trigger_callbacks.find(hash);
        if(it != m_trigger_callbacks.end())
        {
            for(TriggerCallback callback : it->second)
                callback(hash);
        }

        game::g_debug_drawer->DrawScreenText("test", math::Vector(1, 1), mono::Color::BLACK);
    }

    m_triggers_to_emit.clear();
}
