
#include "EntityLogicFactory.h"
#include "IEntityLogic.h"

#include "Enemies/BatController.h"
#include "Enemies/GoblinFireController.h"
#include "Enemies/BlackSquareController.h"
#include "Enemies/CacoDemonController.h"
#include "Enemies/InvaderController.h"
#include "Enemies/InvaderPathController.h"
#include "Enemies/BlobController.h"
#include "Enemies/TurretSpawnerController.h"
#include "Enemies/ExplodableController.h"

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
        MakeController<game::BlackSquareController>,
        MakeController<game::CacodemonController>,
        MakeController<game::InvaderController>,
        MakeController<game::InvaderPathController>,
        MakeController<game::BlobController>,
        MakeController<game::TurretSpawnerController>,
        MakeController<game::ExplodableController>,
    };
}

using namespace game;

EntityLogicFactory::EntityLogicFactory(mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_system_context(system_context)
    , m_event_handler(event_handler)
{ }

IEntityLogic* EntityLogicFactory::CreateLogic(shared::EntityLogicType type, const std::vector<Attribute>& properties, uint32_t entity_id)
{
    if(type == shared::EntityLogicType::INVADER_PATH)
    {
        std::string path_file;
        const bool found_path_property = FindAttribute(PATH_FILE_ATTRIBUTE, properties, path_file, FallbackMode::REQUIRE_ATTRIBUTE);
        if(!found_path_property || path_file.empty())
            return nullptr;

        return new game::InvaderPathController(entity_id, 0, m_system_context, m_event_handler);
    }

    return create_functions[static_cast<uint32_t>(type)](entity_id, m_system_context, m_event_handler);
}
