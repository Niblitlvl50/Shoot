
#pragma once

#include "IGameSystem.h"
#include "Util/ActiveVector.h"

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace game
{
    class IEntityLogic;

    struct EntityLogicComponent
    {
        IEntityLogic* logic;
        uint32_t debug_category;
    };

    struct EntityDebugCategory
    {
        const char* category;
        bool active;
        int reference_counter;
    };

    class EntityLogicSystem : public mono::IGameSystem
    {
    public:

        EntityLogicSystem(size_t n_entities);
        ~EntityLogicSystem();

        void AddLogic(uint32_t entity_id, IEntityLogic* entity_logic);
        void ReleaseLogic(uint32_t entity_id);

        void SetDebugCategory(const char* debug_category, bool activate);
        std::vector<EntityDebugCategory> GetDebugCategories() const;

    private:

        uint32_t Id() const override;
        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::ActiveVector<EntityLogicComponent> m_logics;
        std::unordered_map<uint32_t, EntityDebugCategory> m_hash_to_category;
        std::unordered_set<uint32_t> m_active_categories;
    };
}
