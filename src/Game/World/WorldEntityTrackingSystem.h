
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"

#include <cstdint>
#include <iterator>
#include <vector>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    enum class EntityType : uint32_t
    {
        None,
        Package,
        Boss,
        Loot,
        Quest,
    };

    // Also the names used in entity_tracking_config.json.
    constexpr const char* g_entity_type_strings[] = {
        "None",
        "Package",
        "Boss",
        "Loot",
        "Quest",
    };

    constexpr uint32_t N_ENTITY_TYPES = static_cast<uint32_t>(std::size(g_entity_type_strings));

    struct EntityTrackingComponent
    {
        uint32_t entity_id;
        EntityType type;
        bool enabled;

        uint32_t enable_trigger;
        uint32_t disable_trigger;
        uint32_t enable_callback_id;
        uint32_t disable_callback_id;
    };

    class WorldEntityTrackingSystem : public mono::IGameSystem
    {
    public:

        WorldEntityTrackingSystem(mono::TriggerSystem* trigger_system);

        const char* Name() const override;
        void Begin() override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateEntityTracker(uint32_t entity_id);
        void ReleaseEntityTracker(uint32_t entity_id);
        void UpdateEntityTracker(
            uint32_t entity_id, EntityType type, bool start_enabled, uint32_t enable_trigger, uint32_t disable_trigger);
        void SetTrackerEnabled(uint32_t entity_id, bool enabled);

        void TrackEntity(uint32_t entity_id, EntityType type);
        void ForgetEntity(uint32_t entity_id);

        void SetEntityTypeFilter(EntityType type);
        void ClearProperty(EntityType type);
        void ClearEntityTypeFilter();
        bool IsActiveType(EntityType type) const;

        const std::vector<EntityTrackingComponent>& GetTrackedEntities() const;

    private:

        void RemoveTriggerCallbacks(EntityTrackingComponent& component);

        mono::TriggerSystem* m_trigger_system;
        std::vector<EntityTrackingComponent> m_entities_to_track;
    };
}
