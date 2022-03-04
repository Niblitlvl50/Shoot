
#pragma once

#include "MonoFwd.h"
#include "Math/Vector.h"
#include "StateMachine.h"
#include "Entity/IEntityLogic.h"

namespace game
{
    class EnemyPickupLogic : public IEntityLogic
    {
    public:
        EnemyPickupLogic(uint32_t entity_id, mono::IEntityManager* entity_manager);
        void Update(const mono::UpdateContext& update_context) override;

        const uint32_t m_entity_id;
        mono::IEntityManager* m_entity_manager;

        float m_alive_timer_s;
    };
}
