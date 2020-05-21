
#pragma once

#include "MonoFwd.h"
#include "Math/MathFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Entity/IEntityLogic.h"
#include "Weapons/IWeaponSystem.h"

#include <memory>

namespace game
{
    class CacodemonController : public IEntityLogic
    {
    public:

        CacodemonController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~CacodemonController();

        void Update(const mono::UpdateContext& update_context) override;

    private:

        const uint32_t m_entity_id;
        mono::IBody* m_entity_body;

        std::unique_ptr<IWeaponSystem> m_weapon;
        std::unique_ptr<class TrackingBehaviour> m_tracking_behaviour;

        math::Matrix* m_transform;
    };
}
