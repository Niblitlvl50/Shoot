
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"

#include "Physics/PhysicsFwd.h"

#include <cstdint>
#include <vector>
#include <array>
#include <unordered_map>
#include <functional>
#include <memory>

namespace game
{
    enum class TriggerState
    {
        ENTER,
        EXIT
    };

    struct TriggerComponent
    {
        uint32_t death_trigger_id;
        std::unique_ptr<mono::ICollisionHandler> shape_trigger_handler;
    };

    struct ShapeTrigger
    {
        uint32_t trigger_hash;
        uint32_t collision_mask;
    };

    struct DeathTrigger
    {
        uint32_t trigger_hash;
    };

    using TriggerCallback = std::function<void (uint32_t trigger_id, TriggerState state)>;

    class TriggerSystem : public mono::IGameSystem
    {
    public:

        TriggerSystem(size_t n_triggers, class DamageSystem* damage_system, mono::PhysicsSystem* physics_system);

        TriggerComponent* AllocateTrigger(uint32_t entity_id);
        void ReleaseTrigger(uint32_t entity_id);

        void AddShapeTrigger(uint32_t entity_id, uint32_t trigger_hash, uint32_t collision_mask);
        void AddDeathTrigger(uint32_t entity_id, uint32_t trigger_hash);

        uint32_t RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback);
        void RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id);

        void EmitTrigger(uint32_t trigger_hash, TriggerState state);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        std::vector<TriggerComponent> m_triggers;
        std::vector<bool> m_active;

        class DamageSystem* m_damage_system;
        mono::PhysicsSystem* m_physics_system;

        using TriggerCallbacks = std::array<TriggerCallback, 8>;
        std::unordered_map<uint32_t, TriggerCallbacks> m_trigger_callbacks;

        struct EmitData
        {
            uint32_t hash;
            TriggerState state;
        };
        std::vector<EmitData> m_triggers_to_emit;
    };
}
