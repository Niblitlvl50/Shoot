
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"

namespace game
{
    class LootBoxLogic : public IEntityLogic
    {
    public:

        LootBoxLogic(uint32_t entity_id, class InteractionSystem* interaction_system, mono::IEntityManager* entity_manager);
        void Update(const mono::UpdateContext& update_context) override;

        const uint32_t m_entity_id;
        game::InteractionSystem* m_interaction_system;
        mono::IEntityManager* m_entity_manager;

        float m_alive_timer_s;
    };
}
