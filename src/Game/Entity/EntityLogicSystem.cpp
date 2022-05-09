
#include "EntityLogicSystem.h"
#include "IEntityLogic.h"
#include "System/Hash.h"
#include "Factories.h"
#include "GameDebug.h"

#include <algorithm>
#include <cassert>

using namespace game;

EntityLogicSystem::EntityLogicSystem(size_t n_entities)
    : m_logics(n_entities)
{ }

EntityLogicSystem::~EntityLogicSystem()
{
    /*
    const auto all_is_nullptr = [](IEntityLogic* logic) {
        return logic == nullptr;
    };

    (void)all_is_nullptr;
    assert(std::all_of(m_logics.begin(), m_logics.end(), all_is_nullptr));
    */
}

void EntityLogicSystem::AddLogic(uint32_t entity_id, IEntityLogic* entity_logic)
{
    EntityLogicComponent logic_component;
    logic_component.logic = entity_logic;

    m_logics.Set(entity_id, std::move(logic_component));
}

void EntityLogicSystem::ReleaseLogic(uint32_t entity_id)
{
    EntityLogicComponent* logic_component = m_logics.Get(entity_id);
    delete logic_component->logic;
    logic_component->logic = nullptr;

    m_logics.Release(entity_id);
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

    if(g_draw_entity_logic_debug)
    {
        const auto debug_draw_logic = [](uint32_t index, EntityLogicComponent& logic_component) {
            logic_component.logic->DrawDebugInfo(g_debug_drawer);
        };
        m_logics.ForEach(debug_draw_logic);
    }
}
