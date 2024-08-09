
#pragma once

#include "Weapons/IWeaponModifier.h"

namespace game
{
    class BulletBehaviourModifier : public IWeaponModifier
    {
    public:

        BulletBehaviourModifier(BulletCollisionFlag add_flag);
        BulletConfiguration ModifyBullet(const BulletConfiguration& bullet_config) override;

        const BulletCollisionFlag m_add_flag;
    };
}
