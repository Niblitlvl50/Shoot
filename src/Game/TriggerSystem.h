
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include "Math/Quad.h"
#include "Util/ObjectPool.h"

#include "Physics/PhysicsFwd.h"
#include "TriggerTypes.h"

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
        NONE,
        ENTER,
        EXIT
    };

    struct ShapeTriggerComponent
    {
        uint32_t trigger_hash;
        uint32_t collision_mask;
        std::unique_ptr<mono::ICollisionHandler> shape_trigger_handler;
    };

    struct DeathTriggerComponent
    {
        uint32_t trigger_hash;
        uint32_t death_trigger_id;
    };

    struct AreaEntityTriggerComponent
    {
        uint32_t entity_id;
        uint32_t trigger_hash;
        math::Quad world_bb;
        uint32_t faction;
        int n_entities;
        shared::AreaTriggerOperation operation;
    };

    struct TimeTriggerComponent
    {
        uint32_t entity_id;
        uint32_t trigger_hash;
        float timeout_ms;
        uint32_t time_trigger_id;
    };

    using TriggerCallback = std::function<void (uint32_t trigger_id, TriggerState state)>;

    class TriggerSystem : public mono::IGameSystem
    {
    public:

        TriggerSystem(size_t n_triggers, class DamageSystem* damage_system, mono::PhysicsSystem* physics_system);

        ShapeTriggerComponent* AllocateShapeTrigger(uint32_t entity_id);
        void ReleaseShapeTrigger(uint32_t entity_id);
        void AddShapeTrigger(uint32_t entity_id, uint32_t trigger_hash, uint32_t collision_mask);

        DeathTriggerComponent* AllocateDeathTrigger(uint32_t entity_id);
        void ReleaseDeathTrigger(uint32_t entity_id);
        void AddDeathTrigger(uint32_t entity_id, uint32_t trigger_hash);

        AreaEntityTriggerComponent* AllocateAreaTrigger(uint32_t entity_id);
        void ReleaseAreaTrigger(uint32_t entity_id);
        void AddAreaEntityTrigger(uint32_t entity_id, uint32_t trigger_hash, const math::Quad& world_bb, uint32_t faction, int n_entities);

        TimeTriggerComponent* AllocateTimeTrigger(uint32_t entity_id);
        void ReleaseTimeTrigger(uint32_t entity_id);
        void AddTimeTrigger(uint32_t entity_id, uint32_t trigger_hash, float timeout_ms);

        uint32_t RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback);
        void RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id);

        void EmitTrigger(uint32_t trigger_hash, TriggerState state);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void UpdateAreaEntityTriggers(const mono::UpdateContext& update_context);
        void UpdateTimeTriggers(const mono::UpdateContext& update_context);

        class DamageSystem* m_damage_system;
        mono::PhysicsSystem* m_physics_system;

        using TriggerCallbacks = std::array<TriggerCallback, 8>;
        std::unordered_map<uint32_t, TriggerCallbacks> m_trigger_callbacks;

        std::vector<ShapeTriggerComponent> m_shape_triggers;
        std::vector<bool> m_active_shape_triggers;

        std::vector<DeathTriggerComponent> m_death_triggers;
        std::vector<bool> m_active_death_triggers;

        std::vector<AreaEntityTriggerComponent> m_area_triggers;
        std::vector<bool> m_active_area_triggers;
        uint32_t m_area_trigger_timer;

        std::vector<TimeTriggerComponent> m_time_triggers;
        std::vector<bool> m_active_time_triggers;

        struct EmitData
        {
            uint32_t hash;
            TriggerState state;
        };
        std::vector<EmitData> m_triggers_to_emit;
    };
}
