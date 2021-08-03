
#include "ConditionSystem.h"
#include "System/Hash.h"
#include <algorithm>

using namespace game;

void ConditionSystem::RegisterCondition(uint32_t hash_id, bool initial_value)
{
    Condition& condition = m_conditions[hash_id];
    condition.value = initial_value;
}

void ConditionSystem::UnregisterCondition(uint32_t hash_id)
{
    m_conditions.erase(hash_id);
}

void ConditionSystem::ClearAllConditions()
{
    m_conditions.clear();
}

void ConditionSystem::SetCondition(uint32_t hash_id, bool value)
{
    auto it = m_conditions.find(hash_id);
    if(it != m_conditions.end())
    {
        const bool was_changed = (it->second.value != value);
        it->second.value = value;

        // Perhaps queue this and process the callback in update.
        if(was_changed)
        {
            for(ConditionCallback& callback : it->second.changed_callbacks)
            {
                if(callback)
                    callback(hash_id, value);
            }
        }
    }
}

bool ConditionSystem::IsConditionMet(uint32_t hash_id) const
{
    const auto it = m_conditions.find(hash_id);
    if(it != m_conditions.end())
        return it->second.value;
    
    return false;
}

uint32_t ConditionSystem::RegisterChangedCallback(uint32_t hash_id, const ConditionCallback& callback)
{
    const auto find_free_id = [](const ConditionCallback& callback){
        return callback != nullptr;
    };

    auto it = m_conditions.find(hash_id);
    if(it != m_conditions.end())
    {
        Condition& condition = it->second;
        
        auto callback_it = std::find_if(std::begin(condition.changed_callbacks), std::end(condition.changed_callbacks), find_free_id);
        if(callback_it != std::end(condition.changed_callbacks))
            return std::distance(std::begin(condition.changed_callbacks), callback_it);
    }

    return -1;
}

void ConditionSystem::UnregisterChangedCallback(uint32_t hash_id, uint32_t callback_id)
{
    auto it = m_conditions.find(hash_id);
    if(it != m_conditions.end())
        it->second.changed_callbacks[callback_id] = nullptr;
}

uint32_t ConditionSystem::Id() const
{
    return hash::Hash(Name());
}

const char* ConditionSystem::Name() const
{
    return "conditionsystem";
}

void ConditionSystem::Update(const mono::UpdateContext& update_context)
{

}
