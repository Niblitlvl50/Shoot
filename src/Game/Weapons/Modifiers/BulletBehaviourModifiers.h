
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class BulletBehaviourModifier : public IWeaponModifier
    {
    public:

        BulletBehaviourModifier(const char* name_identifier, BulletCollisionFlag add_flag);

        uint32_t Id() const override;
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;

        const uint32_t m_id;
        const BulletCollisionFlag m_add_flag;
    };
}
