
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include <vector>
#include <functional>
#include <array>
#include <memory>

namespace game
{
    struct DamageRecord
    {
        int health;
        int full_health;
        int multipier;
        uint32_t last_damaged_timestamp;
        bool release_entity_on_death;
        bool is_boss;
        bool is_invincible;
    };

    struct DamageResult
    {
        bool did_damage;
        int health_left;
    };

    enum DamageType
    {
        DESTROYED = 1,
        DAMAGED = 2,
        ALL = DESTROYED | DAMAGED,
    };

    using DamageCallback = std::function<void (uint32_t id, int damage, uint32_t who_did_damage, DamageType type)>;

    class DamageSystem : public mono::IGameSystem
    {
    public:
        DamageSystem(
            size_t num_records,
            mono::TransformSystem* tranform_system,
            mono::SpriteSystem* sprite_system,
            mono::IEntityManager* entity_manager,
            mono::EventHandler* event_handler);

        DamageRecord* CreateRecord(uint32_t id);
        void ReleaseRecord(uint32_t id);
        bool IsAllocated(uint32_t id) const;
        void ReactivateDamageRecord(uint32_t id);
        DamageRecord* GetDamageRecord(uint32_t id);

        uint32_t SetDamageCallback(uint32_t id, uint32_t callback_types, DamageCallback damage_callback);
        void RemoveDamageCallback(uint32_t id, uint32_t callback_id);

        DamageResult ApplyDamage(uint32_t id, int damage, uint32_t id_who_did_damage);
        const std::vector<DamageRecord>& GetDamageRecords() const;

        void PreventReleaseOnDeath(uint32_t id, bool enable);

        template <typename T>
        inline void ForEeach(T&& func)
        {
            for(size_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
            {
                if(m_active[entity_id])
                    func(entity_id, m_damage_records[entity_id]);
            }
        }

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;
        void Destroy() override;

    private:

        size_t FindFreeCallbackIndex(uint32_t id) const;

        mono::TransformSystem* m_transform_system;
        mono::SpriteSystem* m_sprite_system;
        mono::IEntityManager* m_entity_manager;
        mono::EventHandler* m_event_handler;
        uint32_t m_timestamp;
        
        std::vector<DamageRecord> m_damage_records;

        struct DamageCallbackData
        {
            uint32_t callback_types;
            DamageCallback callback;
        };

        using DamageCallbacks = std::array<DamageCallbackData, 8>;
        std::vector<DamageCallbacks> m_damage_callbacks;
        std::vector<bool> m_active;

        std::vector<uint32_t> m_destroyed_but_not_released;
    };
}
