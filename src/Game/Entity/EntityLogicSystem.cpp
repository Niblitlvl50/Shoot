
#include "EntityLogicSystem.h"
#include "IEntityLogic.h"
#include "System/Hash.h"
#include "Factories.h"
#include "Debug/GameDebug.h"

#include <algorithm>
#include <cassert>

using namespace game;

EntityLogicSystem::EntityLogicSystem(size_t n_entities)
    : m_logics(n_entities)
{ }

EntityLogicSystem::~EntityLogicSystem()
{
}

void EntityLogicSystem::AddLogic(uint32_t entity_id, IEntityLogic* entity_logic)
{
    const char* debug_category_string = entity_logic->GetDebugCategory();
    const uint32_t debug_category_hash = hash::Hash(debug_category_string);

    EntityLogicComponent logic_component;
    logic_component.logic = entity_logic;
    logic_component.debug_category = debug_category_hash;

    {
        // Debug stuff
        EntityDebugCategory& debug_category = m_hash_to_category[debug_category_hash];
        debug_category.reference_counter++;
        debug_category.category = debug_category_string;
    }

    m_logics.Set(entity_id, std::move(logic_component));
}

void EntityLogicSystem::ReleaseLogic(uint32_t entity_id)
{
    EntityLogicComponent* logic_component = m_logics.Get(entity_id);

    {
        const uint32_t category_hash = logic_component->debug_category;
        EntityDebugCategory& debug_category = m_hash_to_category[category_hash];
        debug_category.reference_counter--;

        if(debug_category.reference_counter == 0)
            m_hash_to_category.erase(category_hash);
    }

    delete logic_component->logic;
    logic_component->logic = nullptr;

    m_logics.Release(entity_id);
}

void EntityLogicSystem::SetDebugCategory(const char* debug_category, bool activate)
{
    const uint32_t hashed_debug_category = hash::Hash(debug_category);
    if(activate)
        m_active_categories.insert(hashed_debug_category);
    else
        m_active_categories.erase(hashed_debug_category);
}

std::vector<EntityDebugCategory> EntityLogicSystem::GetDebugCategories() const
{
    std::vector<EntityDebugCategory> categories;

    for(const auto& pair : m_hash_to_category)
    {
        EntityDebugCategory debug_category = pair.second;
        debug_category.active =
                (m_active_categories.find(pair.first) != m_active_categories.end());

        categories.push_back(debug_category);
    }

    return categories;
}

uint32_t EntityLogicSystem::Id() const
{
    return hash::Hash(Name());
}

const char* EntityLogicSystem::Name() const
{
    return "entitylogicsystem";
}

void EntityLogicSystem::Update(const mono::UpdateContext& update_context)
{
    const auto update_logic = [&update_context](uint32_t index, EntityLogicComponent& logic_component) {
        logic_component.logic->Update(update_context);
    };
    m_logics.ForEach(update_logic);

    if(!m_active_categories.empty())
    {
        const auto debug_draw_logic = [this](uint32_t index, EntityLogicComponent& logic_component) {
            const bool debug_category_active =
                (m_active_categories.find(logic_component.debug_category) != m_active_categories.end());

            if(debug_category_active)
                logic_component.logic->DrawDebugInfo(g_debug_drawer);
        };
        m_logics.ForEach(debug_draw_logic);
    }
}
