
#pragma once

#include "MonoFwd.h"
#include "Entity/IEntityLogic.h"

namespace game
{
    class BatController : public IEntityLogic
    {
    public:

        BatController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        void Update(const mono::UpdateContext& update_context) override;
    };
}
