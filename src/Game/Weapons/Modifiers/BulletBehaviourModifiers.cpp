
#include "BulletBehaviourModifiers.h"
#include "Weapons/WeaponConfiguration.h"
#include "System/Hash.h"

using namespace game;

BulletBehaviourModifier::BulletBehaviourModifier(const char* name_identifier, BulletCollisionFlag add_flag)
    : m_id(hash::Hash(name_identifier))
    , m_add_flag(add_flag)
{ }

uint32_t BulletBehaviourModifier::Id() const
{
    return m_id;
}

BulletConfiguration BulletBehaviourModifier::ModifyBullet(const BulletConfiguration& bullet_config)
{
    BulletConfiguration config = bullet_config;
    config.bullet_collision_behaviour |= m_add_flag;
    return config;
}
