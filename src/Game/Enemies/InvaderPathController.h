
#pragma once

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"
#include "Entity/IEntityLogic.h"

#include <memory>

namespace game
{
    class InvaderPathController : public IEntityLogic
    {
    public:

        InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        InvaderPathController(uint32_t entity_id, mono::IPathPtr path, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~InvaderPathController();

        void Update(uint32_t delta_ms) override;

    private:

        int m_fire_count;
        int m_fire_cooldown;
        const mono::IPathPtr m_path;

        std::unique_ptr<class PathBehaviour> m_path_behaviour;
        std::unique_ptr<class IWeaponSystem> m_weapon;

        math::Matrix* m_transform;
    };
}
