
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <vector>

namespace game
{
    enum class DestroyedTriggerType : int
    {
        ON_DEATH,
        ON_DESTORYED
    };

    constexpr const char* destroyed_trigger_type_strings[] = {
        "On Death",
        "On Destroyed"
    };

    inline const char* DestroyedTriggerTypeToString(DestroyedTriggerType trigger_type)
    {
        return destroyed_trigger_type_strings[static_cast<int>(trigger_type)];
    }

    struct DestroyedTriggerComponent
    {
        uint32_t entity_id;

        uint32_t trigger_hash;
        DestroyedTriggerType trigger_type;

        // Internal data
        uint32_t callback_id;
    };

    class EntityLifetimeTriggerSystem : public mono::IGameSystem
    {
    public:

        EntityLifetimeTriggerSystem(mono::TriggerSystem* trigger_system, mono::IEntityManager* entity_manager, class DamageSystem* damage_system);

        const char* Name() const override;

        void AllocateDestroyedTrigger(uint32_t entity_id);
        void ReleaseDestroyedTrigger(uint32_t entity_id);
        void AddDestroyedTrigger(uint32_t entity_id, uint32_t trigger_hash, DestroyedTriggerType type);

/*
        template<typename T>
        void ForEachDestroyedTrigger(T&& callable)
        {
            m_destroyed_triggers.ForEach(callable);
        }
*/

        mono::TriggerSystem* m_trigger_system;
        mono::IEntityManager* m_entity_system;
        class DamageSystem* m_damage_system;

        std::vector<DestroyedTriggerComponent> m_destroyed_triggers;
    };
}
