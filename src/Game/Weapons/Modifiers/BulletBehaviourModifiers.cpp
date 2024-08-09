
#include "BulletBehaviourModifiers.h"
#include "Weapons/WeaponConfiguration.h"

using namespace game;

BulletBehaviourModifier::BulletBehaviourModifier(BulletCollisionFlag add_flag)
    : m_add_flag(add_flag)
{ }

BulletConfiguration BulletBehaviourModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration config = bullet_config;
    config.bullet_behaviour |= m_add_flag;
    return config;
}
