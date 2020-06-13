
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"

#include <cstdint>
#include <vector>
#include <string>
#include <array>
#include <unordered_map>
#include <functional>

namespace game
{
    struct TriggerComponent
    {
        uint32_t trigger_hash;
    };

    using TriggerCallback = std::function<void (uint32_t trigger_id)>;

    class TriggerSystem : public mono::IGameSystem
    {
    public:

        TriggerSystem(size_t n_triggers, mono::PhysicsSystem* physics_system);

        TriggerComponent* AllocateTrigger(uint32_t entity_id);
        void ReleaseTrigger(uint32_t entity_id);
        void SetTriggerData(uint32_t entity_id, const TriggerComponent& component_data);

        uint32_t RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback);
        void RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        std::vector<TriggerComponent> m_triggers;
        std::vector<bool> m_active;
        std::vector<std::string> m_debug_names;
        mono::PhysicsSystem* m_physics_system;

        using TriggerCallbacks = std::array<TriggerCallback, 8>;
        std::unordered_map<uint32_t, TriggerCallbacks> m_trigger_callbacks;
    };
}
