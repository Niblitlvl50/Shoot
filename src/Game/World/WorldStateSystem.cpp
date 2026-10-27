
#include "WorldStateSystem.h"
#include "TriggerSystem/TriggerSystem.h"
#include "System/Hash.h"

namespace
{
    bool Compare(int left, int right, game::WorldStateComparison comparison)
    {
        switch(comparison)
        {
        case game::WorldStateComparison::EQUAL:             return left == right;
        case game::WorldStateComparison::NOT_EQUAL:         return left != right;
        case game::WorldStateComparison::LESS:              return left < right;
        case game::WorldStateComparison::LESS_OR_EQUAL:     return left <= right;
        case game::WorldStateComparison::GREATER:           return left > right;
        case game::WorldStateComparison::GREATER_OR_EQUAL:  return left >= right;
        }

        return false;
    }

    template <typename T>
    void RemoveTriggerCallbacks(mono::TriggerSystem* trigger_system, uint32_t entity_id, std::vector<T>& components)
    {
        for(T& component : components)
        {
            if(component.trigger_callback_id != hash::NO_HASH)
                trigger_system->RemoveTriggerCallback(component.trigger_hash, component.trigger_callback_id, entity_id);
            component.trigger_callback_id = hash::NO_HASH;
        }
    }
}

using namespace game;

WorldStateSystem::WorldStateSystem(mono::TriggerSystem* trigger_system)
    : m_trigger_system(trigger_system)
{ }

void WorldStateSystem::SetBool(const std::string& key, bool value)
{
    SetValue(key, value);
}

void WorldStateSystem::SetInt(const std::string& key, int value)
{
    SetValue(key, value);
}

void WorldStateSystem::AddInt(const std::string& key, int value)
{
    SetValue(key, GetInt(key) + value);
}

bool WorldStateSystem::GetBool(const std::string& key, bool default_value) const
{
    const auto it = m_values.find(key);
    if(it == m_values.end())
        return default_value;

    if(const bool* bool_value = std::get_if<bool>(&it->second))
        return *bool_value;
    if(const int* int_value = std::get_if<int>(&it->second))
        return *int_value != 0;

    return default_value;
}

int WorldStateSystem::GetInt(const std::string& key, int default_value) const
{
    const auto it = m_values.find(key);
    if(it == m_values.end())
        return default_value;

    if(const int* int_value = std::get_if<int>(&it->second))
        return *int_value;
    if(const bool* bool_value = std::get_if<bool>(&it->second))
        return *bool_value ? 1 : 0;

    return default_value;
}

bool WorldStateSystem::Has(const std::string& key) const
{
    return m_values.count(key) != 0;
}

void WorldStateSystem::Remove(const std::string& key)
{
    if(m_values.erase(key) != 0)
        m_changed_keys.insert(key);
}

void WorldStateSystem::Clear()
{
    for(const auto& [key, value] : m_values)
        m_changed_keys.insert(key);

    m_values.clear();
}

const WorldStateSystem::WorldStateValues& WorldStateSystem::GetValues() const
{
    return m_values;
}

void WorldStateSystem::SetValues(const WorldStateValues& values)
{
    Clear();

    for(const auto& [key, value] : values)
    {
        const bool is_bool_or_int = std::holds_alternative<bool>(value) || std::holds_alternative<int>(value);
        if(is_bool_or_int)
            SetValue(key, value);
    }
}

void WorldStateSystem::SetValue(const std::string& key, const Variant& value)
{
    m_values[key] = value;
    m_changed_keys.insert(key);
}

// ---------------- Set bool

void WorldStateSystem::AllocateSetBool(uint32_t entity_id)
{
    m_set_bools[entity_id].emplace_back();
}

void WorldStateSystem::ReleaseSetBool(uint32_t entity_id)
{
    const auto it = m_set_bools.find(entity_id);
    if(it == m_set_bools.end())
        return;

    RemoveTriggerCallbacks(m_trigger_system, entity_id, it->second);
    m_set_bools.erase(it);
}

void WorldStateSystem::SetSetBoolData(uint32_t entity_id, const WorldStateSetBoolComponent& data)
{
    const auto it = m_set_bools.find(entity_id);
    if(it == m_set_bools.end() || it->second.empty())
        return;

    // The data is for the component allocated last.
    const size_t component_index = it->second.size() - 1;
    WorldStateSetBoolComponent& component = it->second[component_index];
    if(component.trigger_callback_id != hash::NO_HASH)
        m_trigger_system->RemoveTriggerCallback(component.trigger_hash, component.trigger_callback_id, entity_id);

    component = data;
    component.trigger_callback_id = hash::NO_HASH;

    if(component.trigger_hash != hash::NO_HASH)
    {
        const mono::TriggerCallback set_callback = [this, entity_id, component_index](uint32_t trigger_id) {
            const auto set_it = m_set_bools.find(entity_id);
            if(set_it != m_set_bools.end() && component_index < set_it->second.size())
                ApplySetBool(set_it->second[component_index]);
        };
        component.trigger_callback_id = m_trigger_system->RegisterTriggerCallback(component.trigger_hash, set_callback, entity_id);
    }
}

void WorldStateSystem::ApplySetBool(const WorldStateSetBoolComponent& component)
{
    if(component.key.empty())
        return;

    const bool new_value = (component.operation == WorldStateBoolOperation::TOGGLE) ? !GetBool(component.key) : component.value;
    SetBool(component.key, new_value);
}

// ---------------- Set int

void WorldStateSystem::AllocateSetInt(uint32_t entity_id)
{
    m_set_ints[entity_id].emplace_back();
}

void WorldStateSystem::ReleaseSetInt(uint32_t entity_id)
{
    const auto it = m_set_ints.find(entity_id);
    if(it == m_set_ints.end())
        return;

    RemoveTriggerCallbacks(m_trigger_system, entity_id, it->second);
    m_set_ints.erase(it);
}

void WorldStateSystem::SetSetIntData(uint32_t entity_id, const WorldStateSetIntComponent& data)
{
    const auto it = m_set_ints.find(entity_id);
    if(it == m_set_ints.end() || it->second.empty())
        return;

    // The data is for the component allocated last.
    const size_t component_index = it->second.size() - 1;
    WorldStateSetIntComponent& component = it->second[component_index];
    if(component.trigger_callback_id != hash::NO_HASH)
        m_trigger_system->RemoveTriggerCallback(component.trigger_hash, component.trigger_callback_id, entity_id);

    component = data;
    component.trigger_callback_id = hash::NO_HASH;

    if(component.trigger_hash != hash::NO_HASH)
    {
        const mono::TriggerCallback set_callback = [this, entity_id, component_index](uint32_t trigger_id) {
            const auto set_it = m_set_ints.find(entity_id);
            if(set_it != m_set_ints.end() && component_index < set_it->second.size())
                ApplySetInt(set_it->second[component_index]);
        };
        component.trigger_callback_id = m_trigger_system->RegisterTriggerCallback(component.trigger_hash, set_callback, entity_id);
    }
}

void WorldStateSystem::ApplySetInt(const WorldStateSetIntComponent& component)
{
    if(component.key.empty())
        return;

    if(component.operation == WorldStateIntOperation::ADD)
        AddInt(component.key, component.value);
    else
        SetInt(component.key, component.value);
}

// ---------------- Conditions

void WorldStateSystem::AllocateBoolCondition(uint32_t entity_id)
{
    m_bool_conditions[entity_id] = WorldStateBoolConditionComponent();
}

void WorldStateSystem::ReleaseBoolCondition(uint32_t entity_id)
{
    m_bool_conditions.erase(entity_id);
}

void WorldStateSystem::SetBoolConditionData(uint32_t entity_id, const WorldStateBoolConditionComponent& data)
{
    const auto it = m_bool_conditions.find(entity_id);
    if(it == m_bool_conditions.end())
        return;

    // Checked again in the next update, as when the level starts.
    it->second = data;
    it->second.evaluated = false;
}

void WorldStateSystem::AllocateIntCondition(uint32_t entity_id)
{
    m_int_conditions[entity_id] = WorldStateIntConditionComponent();
}

void WorldStateSystem::ReleaseIntCondition(uint32_t entity_id)
{
    m_int_conditions.erase(entity_id);
}

void WorldStateSystem::SetIntConditionData(uint32_t entity_id, const WorldStateIntConditionComponent& data)
{
    const auto it = m_int_conditions.find(entity_id);
    if(it == m_int_conditions.end())
        return;

    it->second = data;
    it->second.evaluated = false;
}

template <typename T>
void WorldStateSystem::UpdateCondition(T& condition, bool result)
{
    const bool first_evaluation = !condition.evaluated;
    const bool result_changed = (result != condition.last_result);

    condition.evaluated = true;
    condition.last_result = result;

    if(!first_evaluation && !result_changed)
        return;

    const uint32_t trigger_hash = result ? condition.true_trigger_hash : condition.false_trigger_hash;
    if(trigger_hash != hash::NO_HASH)
        m_trigger_system->EmitTrigger(trigger_hash);
}

const char* WorldStateSystem::Name() const
{
    return "worldstatesystem";
}

void WorldStateSystem::Update(const mono::UpdateContext& update_context)
{
    for(auto& [entity_id, condition] : m_bool_conditions)
    {
        if(condition.evaluated && m_changed_keys.count(condition.key) == 0)
            continue;

        UpdateCondition(condition, GetBool(condition.key) == condition.value);
    }

    for(auto& [entity_id, condition] : m_int_conditions)
    {
        if(condition.evaluated && m_changed_keys.count(condition.key) == 0)
            continue;

        UpdateCondition(condition, Compare(GetInt(condition.key), condition.value, condition.comparison));
    }

    m_changed_keys.clear();
}
