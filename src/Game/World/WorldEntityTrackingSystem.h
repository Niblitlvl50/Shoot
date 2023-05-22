
#pragma once

#include "IGameSystem.h"

#include <cstdint>
#include <vector>

#define ENUM_BIT(n) (1 << (n))

namespace game
{
    enum class EntityType : uint32_t
    {
        None    = 0,
        Package = ENUM_BIT(0),
        Boss    = ENUM_BIT(1),
        Loot    = ENUM_BIT(2),
        All     = (~(uint32_t)0),
    };

    constexpr const char* g_entity_type_strings[] = {
        "None",
        "Package",
        "Boss",
        "Loot",
    };

    struct EntityTrackingComponent
    {
        uint32_t entity_id;
        EntityType type;
    };

    class WorldEntityTrackingSystem : public mono::IGameSystem
    {
    public:

        WorldEntityTrackingSystem();

        const char* Name() const override;
        void Begin() override;
        void Update(const mono::UpdateContext& update_context) override;

        void AllocateEntityTracker(uint32_t entity_id);
        void ReleaseEntityTracker(uint32_t entity_id);
        void UpdateEntityTracker(uint32_t entity_id, EntityType type);

        void TrackEntity(uint32_t entity_id, EntityType type);
        void ForgetEntity(uint32_t entity_id);

        void SetEntityTypeFilter(EntityType type);
        void ClearProperty(EntityType type);
        void ClearEntityTypeFilter();
        bool IsActiveType(EntityType type) const;

        const std::vector<EntityTrackingComponent>& GetTrackedEntities() const;

    private:

        uint32_t m_type_filter;
        std::vector<EntityTrackingComponent> m_entities_to_track;
    };
}
