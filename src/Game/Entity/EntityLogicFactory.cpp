
#include "EntityLogicFactory.h"
#include "IEntityLogic.h"

#include "Enemies/BatController.h"
#include "Enemies/BeastController.h"
#include "Enemies/BlackSquareController.h"
#include "Enemies/CacoDemonController.h"
#include "Enemies/InvaderController.h"
#include "Enemies/InvaderPathController.h"

#include "Paths/IPath.h"
#include "Paths/PathFactory.h"

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
        MakeController<game::BeastController>,
        MakeController<game::BlackSquareController>,
        MakeController<game::CacodemonController>,
        MakeController<game::InvaderController>,
        MakeController<game::InvaderPathController>,
    };
}

using namespace game;

EntityLogicFactory::EntityLogicFactory(mono::SystemContext* system_context, mono::EventHandler& event_handler)
    : m_system_context(system_context)
    , m_event_handler(event_handler)
{ }

IEntityLogic* EntityLogicFactory::CreateLogic(EntityLogicType type, uint32_t entity_id)
{
    return create_functions[static_cast<uint32_t>(type)](entity_id, m_system_context, m_event_handler);
}

IEntityLogic* EntityLogicFactory::CreatePathInvaderLogic(const char* path_file, uint32_t entity_id)
{
    return new game::InvaderPathController(entity_id, mono::CreatePath(path_file), m_system_context, m_event_handler);
}
