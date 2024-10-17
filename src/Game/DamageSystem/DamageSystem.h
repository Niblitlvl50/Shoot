
#pragma once

#include "IGameSystem.h"
#include "DamageSystemTypes.h"

#include "MonoFwd.h"

#include <vector>
#include <array>
#include <memory>
#include <string>
#include <unordered_map>


namespace game
{
    struct DamageRecord
    {
        int health;
        int full_health;
        float multipier;
        uint32_t last_damaged_timestamp;
        bool release_entity_on_death;
        bool is_boss;
        bool is_invincible;
    };

    struct DamageEvent
    {
        uint32_t id_damaged_entity;
        uint32_t id_who_did_damage;
        uint32_t weapon_identifier;
        int damage;
        DamageType damage_result;
    };

    struct DamageResult
    {
        bool did_damage;
        int health_left;
    };

    struct ShockwaveComponent
    {
        uint32_t trigger_hash;
        float radius;
        float magnitude;
        int damage;

        // Internal
        uint32_t trigger_handle;
    };

    class DamageSystem : public mono::IGameSystem
    {
    public:
        DamageSystem(
            size_t num_records,
            mono::TransformSystem* tranform_system,
            mono::SpriteSystem* sprite_system,
            mono::PhysicsSystem* physics_system,
            mono::IEntityManager* entity_manager,
            mono::TriggerSystem* trigger_system);

        DamageRecord* CreateRecord(uint32_t id);
        void ReleaseRecord(uint32_t id);
        bool IsAllocated(uint32_t id) const;
        void ReactivateDamageRecord(uint32_t id);
        DamageRecord* GetDamageRecord(uint32_t id);

        ShockwaveComponent* CreateShockwaveComponent(uint32_t entity_id);
        void ReleaseShockwaveComponent(uint32_t entity_id);
        void UpdateShockwaveComponent(uint32_t entity_id, uint32_t trigger, float radius, float magnitude, int damage);

        void SetDamageFilter(uint32_t id, DamageFilter damage_filter);
        void ClearDamageFilter(uint32_t id);

        uint32_t SetDamageCallback(uint32_t id, uint32_t callback_types, DamageCallback damage_callback);
        void RemoveDamageCallback(uint32_t id, uint32_t callback_id);

        uint32_t SetGlobalDamageCallback(uint32_t callback_types, DamageCallback damage_callback);
        void RemoveGlobalDamageCallback(uint32_t callback_id);

        DamageResult ApplyDamage(uint32_t id_damaged_entity, uint32_t id_who_did_damage, uint32_t weapon_identifier, int damage);
        void GainHealth(uint32_t id, int health_gain);
        const std::vector<DamageRecord>& GetDamageRecords() const;
        const std::vector<DamageEvent>& GetDamageEventsThisFrame() const;

        void ApplyShockwave(uint32_t entity_id);

        void PreventReleaseOnDeath(uint32_t id, bool enable);

        bool IsInvincible(uint32_t id) const;
        void SetInvincible(uint32_t id, bool invincible);

        void SetDamageMultiplier(uint32_t id, float multiplier);
        float GetDamageMultiplier(uint32_t id) const;

        bool IsBoss(uint32_t id) const;

        template <typename T>
        inline void ForEeach(T&& func)
        {
            for(uint32_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
            {
                if(m_active[entity_id])
                    func(entity_id, m_damage_records[entity_id]);
            }
        }

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void PostUpdate() override;

    private:

        struct DamageCallbackData
        {
            uint32_t callback_types;
            DamageCallback callback;
        };
        using DamageCallbacks = std::array<DamageCallbackData, 8>;

        uint32_t FindFreeCallbackIndex(const DamageCallbacks& callbacks) const;

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::PhysicsSystem* m_physics_system;
        mono::IEntityManager* m_entity_manager;
        mono::TriggerSystem* m_trigger_system;
        uint32_t m_timestamp;
        
        std::vector<DamageRecord> m_damage_records;
        std::vector<DamageCallbacks> m_damage_callbacks;
        std::vector<DamageFilter> m_damage_filters;
        std::vector<bool> m_active;

        std::vector<std::string> m_death_entities;
        DamageCallbacks m_global_damage_callbacks;

        std::vector<DamageEvent> m_damage_events;

        std::unordered_map<uint32_t, ShockwaveComponent> m_shockwave_components;
    };
}
