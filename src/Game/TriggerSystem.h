
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

    struct TriggerComponent
    {
        uint32_t death_trigger_id;
        uint32_t area_trigger_id;
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

    struct AreaEntityTrigger
    {
        uint32_t entity_id;
        uint32_t trigger_hash;
        math::Quad world_bb;
        uint32_t faction;
        int n_entities;
        shared::AreaTriggerOperation operation;
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
        void AddAreaEntityTrigger(uint32_t entity_id, uint32_t trigger_hash, const math::Quad& world_bb, uint32_t faction, int n_entities);

        uint32_t RegisterTriggerCallback(uint32_t trigger_hash, TriggerCallback callback);
        void RemoveTriggerCallback(uint32_t trigger_hash, uint32_t callback_id);

        void EmitTrigger(uint32_t trigger_hash, TriggerState state);

        uint32_t Id() const override;
        const char* Name() const override;
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        void UpdateAreaEntityTriggers(const mono::UpdateContext& update_context);

        std::vector<TriggerComponent> m_triggers;
        std::vector<bool> m_active;

        class DamageSystem* m_damage_system;
        mono::PhysicsSystem* m_physics_system;

        using TriggerCallbacks = std::array<TriggerCallback, 8>;
        std::unordered_map<uint32_t, TriggerCallbacks> m_trigger_callbacks;

        mono::ObjectPool<AreaEntityTrigger> m_area_triggers;
        std::vector<AreaEntityTrigger*> m_active_area_triggers;
        uint32_t m_area_trigger_timer;

        struct EmitData
        {
            uint32_t hash;
            TriggerState state;
        };
        std::vector<EmitData> m_triggers_to_emit;
    };
}
