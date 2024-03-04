
#pragma once

#include "MonoFwd.h"
#include "IGameSystem.h"
#include "Util/ActiveVector.h"
#include "EntityLogicTypes.h"

#include <cstddef>
#include <vector>
#include <unordered_map>
#include <unordered_set>

struct Attribute;

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

        EntityLogicSystem(uint32_t n_entities, mono::SystemContext* system_context, mono::EventHandler* event_handler);
        ~EntityLogicSystem();

        void AddLogic(uint32_t entity_id, IEntityLogic* entity_logic);
        void ReleaseLogic(uint32_t entity_id);

        void SetDebugCategory(const char* debug_category, bool activate);
        std::vector<EntityDebugCategory> GetDebugCategories() const;

        IEntityLogic* CreateLogic(EntityLogicType type, const std::vector<Attribute>& properties, uint32_t entity_id);

        template <typename T>
        void ForEach(T&& callback)
        {
            m_logics.ForEach(callback);
        }

    private:

        const char* Name() const override;
        void Update(const mono::UpdateContext& update_context) override;

        mono::SystemContext* m_system_context;
        mono::EventHandler* m_event_handler;

        mono::ActiveVector<EntityLogicComponent> m_logics;
        std::unordered_map<uint32_t, EntityDebugCategory> m_hash_to_category;
        std::unordered_set<uint32_t> m_active_categories;
    };
}
