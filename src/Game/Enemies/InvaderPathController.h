
#pragma once

#include "MonoPtrFwd.h"
#include "Math/MathFwd.h"
#include "Entity/IEntityLogic.h"
#include "Rendering/RenderFwd.h"
#include "Physics/PhysicsFwd.h"
#include "Weapons/WeaponFwd.h"

namespace game
{
    class InvaderPathController : public IEntityLogic
    {
    public:

        InvaderPathController(uint32_t entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        InvaderPathController(uint32_t entity_id, uint32_t path_entity_id, mono::SystemContext* system_context, mono::EventHandler& event_handler);
        ~InvaderPathController();

        void Update(const mono::UpdateContext& update_context) override;

    private:

        int m_fire_count;
        float m_fire_cooldown_s;
        mono::IPathPtr m_path;

        std::unique_ptr<class PathBehaviour> m_path_behaviour;
        IWeaponPtr m_weapon;

        math::Matrix* m_transform;
        mono::ISprite* m_sprite;
        mono::IBody* m_body;
    };
}
