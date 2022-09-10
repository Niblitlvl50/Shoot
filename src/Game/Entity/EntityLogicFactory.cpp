
#include "EntityLogicFactory.h"
#include "IEntityLogic.h"

#include "Enemies/BatController.h"
#include "Enemies/GoblinFireController.h"
#include "Enemies/EyeMonsterController.h"
#include "Enemies/CacoDemonController.h"
#include "Enemies/FlyingMonsterController.h"
#include "Enemies/InvaderPathController.h"
#include "Enemies/BlobController.h"
#include "Enemies/TurretSpawnerController.h"
#include "Enemies/ExplodableController.h"
#include "Enemies/FlamingSkullBossController.h"
#include "Enemies/ImpController.h"

#include "SystemContext.h"
#include "EntitySystem/IEntityManager.h"
#include "Paths/PathFactory.h"
#include "Component.h"

namespace
{
    using CreateLogicFunc =
        game::IEntityLogic*(*)(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);

    template <typename T>
    game::IEntityLogic* MakeController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler)
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
        MakeController<game::TurretSpawnerController>,
        MakeController<game::ExplodableController>,
        MakeController<game::FlamingSkullBossController>,
        MakeController<game::ImpController>,
    };
}

using namespace game;

EntityLogicFactory::EntityLogicFactory(mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_system_context(system_context)
    , m_event_handler(event_handler)
{ }

IEntityLogic* EntityLogicFactory::CreateLogic(EntityLogicType type, const std::vector<Attribute>& properties, uint32_t entity_id)
{
    if(type == EntityLogicType::INVADER_PATH)
    {
        uint32_t entity_reference;
        const bool found_path_property = FindAttribute(ENTITY_REFERENCE_ATTRIBUTE, properties, entity_reference, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_path_property)
            return nullptr;

        mono::IEntityManager* entity_manager = m_system_context->GetSystem<mono::IEntityManager>();
        const uint32_t path_entity_id = entity_manager->GetEntityIdFromUuid(entity_reference);

        return new game::InvaderPathController(entity_id, path_entity_id, m_system_context, m_event_handler);
    }

    return create_functions[static_cast<uint32_t>(type)](entity_id, m_system_context, m_event_handler);
}
