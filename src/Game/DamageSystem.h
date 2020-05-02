
#pragma once

#include "IGameSystem.h"
#include "MonoFwd.h"
#include <vector>
#include <functional>
#include <array>

class IEntityManager;

namespace game
{
    struct DamageRecord
    {
        int health;
        int full_health;
        int multipier;
        int score;
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
        DamageSystem(size_t num_records, IEntityManager* entity_manager, mono::EventHandler* event_handler);

        DamageRecord* CreateRecord(uint32_t id);
        uint32_t SetDestroyedCallback(uint32_t id, DestroyedCallback destroyed_callback);
        void RemoveCallback(uint32_t id, uint32_t callback_id);
        void ReleaseRecord(uint32_t id);

        DamageRecord* GetDamageRecord(uint32_t id);

        DamageResult ApplyDamage(uint32_t id, int damage, uint32_t id_who_did_damage);
        const std::vector<DamageRecord>& GetDamageRecords() const;

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
        uint32_t Capacity() const override;
        void Update(const mono::UpdateContext& update_context) override;

    private:

        size_t FindFreeCallbackIndex(uint32_t id) const;

        IEntityManager* m_entity_manager;
        mono::EventHandler* m_event_handler;
        uint32_t m_elapsed_time;
        
        std::vector<DamageRecord> m_damage_records;

        using DestroyedCallbacks = std::array<DestroyedCallback, 8>;
        std::vector<DestroyedCallbacks> m_destroyed_callbacks;
        std::vector<bool> m_active;
    };
}
