
#include "EntityLogicSystem.h"
#include "IEntityLogic.h"
#include "System/Hash.h"
#include "Debug/IDebugDrawer.h"

#include "EntitySystem/ObjectAttribute.h"


#include "Enemies/BatController.h"
#include "Enemies/BirdController.h"
#include "Enemies/GoblinFireController.h"
#include "Enemies/EyeMonsterController.h"
#include "Enemies/DemonBossController.h"
#include "Enemies/FlyingMonsterController.h"
#include "Enemies/InvaderPathController.h"
#include "Enemies/BlobController.h"
#include "Enemies/ExplodableController.h"
#include "Enemies/FlamingSkullBossController.h"
#include "Enemies/ImpController.h"
#include "Enemies/BombThrowerController.h"
#include "Enemies/DemonMinionController.h"

#include "World/ReactivePropLogic.h"


#include <algorithm>
#include <cassert>


namespace
{
    class NullLogic : public game::IEntityLogic
    {
    public:

        NullLogic(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
        { }

        void Update(const mono::UpdateContext& update_context) override
        { }
    };

    using CreateLogicFunc =
        game::IEntityLogic*(*)(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler);

    template <typename T>
    game::IEntityLogic* MakeController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    {
        return new T(entity_id, system_context, event_handler);
    }

    constexpr CreateLogicFunc create_functions[] = {
        MakeController<game::BatController>,
        MakeController<game::GoblinFireController>,
        MakeController<game::EyeMonsterController>,
        MakeController<game::DemonBossController>,
        MakeController<game::FlyingMonsterController>,
        MakeController<game::InvaderPathController>,
        MakeController<game::BlobController>,
        MakeController<game::BirdController>,
        MakeController<game::ExplodableController>,
        MakeController<game::FlamingSkullBossController>,
        MakeController<game::ImpController>,
        MakeController<game::BombThrowerController>,
        MakeController<game::DemonMinionController>,
        MakeController<game::ReactivePropLogic>,
    };
}

using namespace game;

EntityLogicSystem::EntityLogicSystem(
    uint32_t n_entities, mono::SystemContext* system_context, mono::EventHandler* event_handler)
    : m_system_context(system_context)
    , m_event_handler(event_handler)
    , m_logics(n_entities)
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

IEntityLogic* EntityLogicSystem::CreateLogic(EntityLogicType type, const std::vector<Attribute>& properties, uint32_t entity_id)
{
    return create_functions[static_cast<uint32_t>(type)](entity_id, m_system_context, m_event_handler);
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
