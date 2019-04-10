
#pragma once

#include "MonoPtrFwd.h"
#include "Physics/IBody.h"

#include "Entity/IEntityLogic.h"
#include "WeaponConfiguration.h"

namespace game
{
    class BulletLogic : public game::IEntityLogic, public mono::ICollisionHandler
    {
    public:

        BulletLogic(uint32_t entity_id, const BulletConfiguration& config);
        void Update(uint32_t delta_ms) override;
        void OnCollideWith(mono::IBody* body, const math::Vector& collision_point, uint32_t categories) override;

        const uint32_t m_entity_id;
        BulletImpactCallback m_collision_callback;
        int m_life_span;
        mono::ISoundPtr m_sound;
    };
}
