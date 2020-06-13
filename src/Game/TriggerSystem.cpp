
#include "TriggerSystem.h"
#include "Util/Hash.h"
#include "Physics/IShape.h"
#include "Physics/PhysicsSystem.h"

using namespace game;

TriggerSystem::TriggerSystem(size_t n_triggers, mono::PhysicsSystem* physics_system)
    : m_triggers(n_triggers)
    , m_active(n_triggers, false)
    , m_physics_system(physics_system)
{ }

TriggerComponent* TriggerSystem::AllocateTrigger(uint32_t entity_id)
{
    assert(!m_active[entity_id]);
    m_active[entity_id] = true;
    return &m_triggers[entity_id];
}

void TriggerSystem::ReleaseTrigger(uint32_t entity_id)
{
    m_active[entity_id] = false;
}

void TriggerSystem::SetTriggerData(uint32_t entity_id, const TriggerComponent& component_data)
{
    m_triggers[entity_id] = component_data;

    const std::vector<mono::IShape*>& shapes = m_physics_system->GetShapesAttachedToBody(entity_id);
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

}
