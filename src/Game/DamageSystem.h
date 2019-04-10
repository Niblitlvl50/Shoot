
#pragma once

#include "IGameSystem.h"
#include <vector>
#include <functional>

class IEntityManager;

namespace game
{
    struct DamageRecord
    {
        int health;
        int multipier;
        uint32_t strong_against;
        uint32_t weak_against;
        uint32_t last_damaged_timestamp;
    };

    struct DamageResult
    {
        int health_left;
    };

    using DestroyedCallback = std::function<void (uint32_t id)>;

    class DamageSystem : public mono::IGameSystem
    {
    public:
        DamageSystem(size_t num_records, IEntityManager* entity_manager);

        DamageRecord* CreateRecord(uint32_t id);
        DamageRecord* CreateRecord(uint32_t id, DestroyedCallback destroyed_callback);
        void SetDestroyedCallback(uint32_t id, DestroyedCallback destroyed_callback);
        void ReleaseRecord(uint32_t id);

        DamageRecord* GetDamageRecord(uint32_t id);

        DamageResult ApplyDamage(uint32_t id, int damage);
        const std::vector<DamageRecord>& GetDamageRecords() const;

        template <typename T>
        inline void ForEeachRecord(T&& func)
        {
            for(size_t entity_id = 0; entity_id < m_damage_records.size(); ++entity_id)
            {
                if(m_active[entity_id])
                    func(entity_id, m_damage_records[entity_id]);
            }
        }

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(uint32_t delta_ms) override;

    private:

        IEntityManager* m_entity_manager;
        uint32_t m_elapsed_time;
        
        std::vector<DamageRecord> m_damage_records;
        std::vector<DestroyedCallback> m_destroyed_callbacks;
        std::vector<bool> m_active;
    };
}
