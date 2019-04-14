
#include "EntityLogicSystem.h"
#include "IEntityLogic.h"
#include "Hash.h"

#include <algorithm>
#include <cassert>

using namespace game;

EntityLogicSystem::EntityLogicSystem(size_t n_entities)
    : m_logics(n_entities, nullptr)
{ }

EntityLogicSystem::~EntityLogicSystem()
{
    const auto all_is_nullptr = [](IEntityLogic* logic) {
        return logic == nullptr;
    };

    assert(std::all_of(m_logics.begin(), m_logics.end(), all_is_nullptr));
}

void EntityLogicSystem::AddLogic(uint32_t entity_id, IEntityLogic* entity_logic)
{
    assert(m_logics[entity_id] == nullptr);
    m_logics[entity_id] = entity_logic;
}

void EntityLogicSystem::ReleaseLogic(uint32_t entity_id)
{
    assert(m_logics[entity_id] != nullptr);

    IEntityLogic* logic = m_logics[entity_id];
    delete logic;

    m_logics[entity_id] = nullptr;
}

uint32_t EntityLogicSystem::Id() const
{
    return mono::Hash(Name());
}

const char* EntityLogicSystem::Name() const
{
    return "entitylogicsystem";
}

uint32_t EntityLogicSystem::Capacity() const
{
    return m_logics.size();
}

void EntityLogicSystem::Update(uint32_t delta_ms)
{
    for(IEntityLogic* logic : m_logics)
    {
        if(logic)
            logic->Update(delta_ms);
    }
}
