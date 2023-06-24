
#pragma once

#include "IGameSystem.h"

#include <cstdint>
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

        std::vector<EntityTrackingComponent> m_entities_to_track;
    };
}
