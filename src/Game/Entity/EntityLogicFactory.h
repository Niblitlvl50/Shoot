
#pragma once

#include "MonoFwd.h"
#include "EntityLogicTypes.h"

namespace game
{
    class EntityLogicFactory
    {
    public:

        EntityLogicFactory(mono::SystemContext* system_context, mono::EventHandler& event_handler);
        class IEntityLogic* CreateLogic(EntityLogicType type, uint32_t entity_id);
        class IEntityLogic* CreatePathInvaderLogic(const char* path_file, uint32_t entity_id);

    private:
        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;
    };
}
