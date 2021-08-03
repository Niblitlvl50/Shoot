
#pragma once

#include "IGameSystem.h"

#include <unordered_map>
#include <functional>

namespace game
{
    // hash of the conditions name and the new value of the condition
    using ConditionCallback = std::function<void (uint32_t id, bool value)>;

    class ConditionSystem : public mono::IGameSystem
    {
    public:

        void RegisterCondition(uint32_t hash_id, bool initial_value);
        void UnregisterCondition(uint32_t hash_id);
        void ClearAllConditions();

        void SetCondition(uint32_t hash_id, bool value);
        bool IsConditionMet(uint32_t hash_id) const;

        uint32_t RegisterChangedCallback(uint32_t hash_id, const ConditionCallback& callback);
        void UnregisterChangedCallback(uint32_t hash_id, uint32_t callback_id);

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        struct Condition
        {
            bool value;
            ConditionCallback changed_callbacks[8];
        };
        std::unordered_map<uint32_t, Condition> m_conditions;
    };
}
