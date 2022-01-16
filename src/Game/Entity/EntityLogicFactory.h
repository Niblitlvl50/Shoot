
#pragma once

#include "MonoFwd.h"
#include "EntityLogicTypes.h"
#include <vector>

struct Attribute;

namespace game
{
    class EntityLogicFactory
    {
    public:

        EntityLogicFactory(mono::SystemContext* system_context, mono::EventHandler& event_handler);
        class IEntityLogic* CreateLogic(EntityLogicType type, const std::vector<Attribute>& properties, uint32_t entity_id);

    private:
        mono::SystemContext* m_system_context;
        mono::EventHandler& m_event_handler;
    };
}
