
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "EntitySystem/ObjectAttribute.h"
#include "System/Hash.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace game
{
    enum class WorldStateBoolOperation : int
    {
        SET,
        TOGGLE,
    };

    constexpr const char* g_world_state_bool_operation_strings[] = {
        "Set",
        "Toggle",
    };

    enum class WorldStateIntOperation : int
    {
        SET,
        ADD,
    };

    constexpr const char* g_world_state_int_operation_strings[] = {
        "Set",
        "Add",
    };

    enum class WorldStateComparison : int
    {
        EQUAL,
        NOT_EQUAL,
        LESS,
        LESS_OR_EQUAL,
        GREATER,
        GREATER_OR_EQUAL,
    };

    constexpr const char* g_world_state_comparison_strings[] = {
        "==",
        "!=",
        "<",
        "<=",
        ">",
        ">=",
    };

    // Sets a bool world state value when a trigger fires.
    struct WorldStateSetBoolComponent
    {
        std::string key;
        WorldStateBoolOperation operation = WorldStateBoolOperation::SET;
        bool value = false;
        uint32_t trigger_hash = hash::NO_HASH;

        // Internal
        uint32_t trigger_callback_id = hash::NO_HASH;
    };

    // Sets or adds to an int world state value when a trigger fires.
    struct WorldStateSetIntComponent
    {
        std::string key;
        WorldStateIntOperation operation = WorldStateIntOperation::SET;
        int value = 0;
        uint32_t trigger_hash = hash::NO_HASH;

        // Internal
        uint32_t trigger_callback_id = hash::NO_HASH;
    };

    // The conditions are checked when the level starts and whenever their key changes, and emit
    // true_trigger or false_trigger with the result. After the first check they only emit when the
    // result changes.
    struct WorldStateBoolConditionComponent
    {
        std::string key;
        bool value = true;
        uint32_t true_trigger_hash = hash::NO_HASH;
        uint32_t false_trigger_hash = hash::NO_HASH;

        // Internal
        bool evaluated = false;
        bool last_result = false;
    };

    struct WorldStateIntConditionComponent
    {
        std::string key;
        WorldStateComparison comparison = WorldStateComparison::EQUAL;
        int value = 0;
        uint32_t true_trigger_hash = hash::NO_HASH;
        uint32_t false_trigger_hash = hash::NO_HASH;

        // Internal
        bool evaluated = false;
        bool last_result = false;
    };

    // Named values that persist between levels and are saved with the player's save slot. Values are
    // stored as Variants but only bool and int are used. A key that has never been set reads as false
    // or 0.
    class WorldStateSystem : public mono::IGameSystem
    {
    public:

        using WorldStateValues = std::unordered_map<std::string, mono::Variant>;

        WorldStateSystem(mono::TriggerSystem* trigger_system);

        void SetBool(const std::string& key, bool value);
        void SetInt(const std::string& key, int value);
        void AddInt(const std::string& key, int value);

        bool GetBool(const std::string& key, bool default_value = false) const;
        int GetInt(const std::string& key, int default_value = 0) const;

        bool Has(const std::string& key) const;
        void Remove(const std::string& key);
        void Clear();

        // For saving and loading, values that aren't a bool or an int are skipped when set.
        const WorldStateValues& GetValues() const;
        void SetValues(const WorldStateValues& values);

        // An entity can have several set components of each kind, each Allocate call appends one and
        // the following Set...Data call fills in the one just added. Release removes all of that kind.
        void AllocateSetBool(uint32_t entity_id);
        void ReleaseSetBool(uint32_t entity_id);
        void SetSetBoolData(uint32_t entity_id, const WorldStateSetBoolComponent& data);

        void AllocateSetInt(uint32_t entity_id);
        void ReleaseSetInt(uint32_t entity_id);
        void SetSetIntData(uint32_t entity_id, const WorldStateSetIntComponent& data);

        void AllocateBoolCondition(uint32_t entity_id);
        void ReleaseBoolCondition(uint32_t entity_id);
        void SetBoolConditionData(uint32_t entity_id, const WorldStateBoolConditionComponent& data);

        void AllocateIntCondition(uint32_t entity_id);
        void ReleaseIntCondition(uint32_t entity_id);
        void SetIntConditionData(uint32_t entity_id, const WorldStateIntConditionComponent& data);

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void SetValue(const std::string& key, const mono::Variant& value);

        void ApplySetBool(const WorldStateSetBoolComponent& component);
        void ApplySetInt(const WorldStateSetIntComponent& component);

        // Checks a condition if it hasn't been or its key changed, and emits its trigger when needed.
        template <typename T>
        void UpdateCondition(T& condition, bool result);

        mono::TriggerSystem* m_trigger_system;

        // Persists between levels, the components below are per level.
        WorldStateValues m_values;

        std::unordered_map<uint32_t, std::vector<WorldStateSetBoolComponent>> m_set_bools;
        std::unordered_map<uint32_t, std::vector<WorldStateSetIntComponent>> m_set_ints;
        std::unordered_map<uint32_t, WorldStateBoolConditionComponent> m_bool_conditions;
        std::unordered_map<uint32_t, WorldStateIntConditionComponent> m_int_conditions;

        // Keys changed since the conditions were last checked.
        std::unordered_set<std::string> m_changed_keys;
    };
}
