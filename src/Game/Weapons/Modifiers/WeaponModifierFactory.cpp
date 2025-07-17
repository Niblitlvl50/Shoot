
#include "WeaponModifierFactory.h"
#include "Weapons/Modifiers/DamageModifier.h"
#include "Weapons/Modifiers/BulletBehaviourModifiers.h"


game::IWeaponModifier* game::WeaponModifierFactory::CreateModifierForWeaponAndLevel(
    uint32_t weapon_identifier_hash, int level)
{
    switch(level)
    {
    case 0:
        break;

    case 1:
        return new DamageModifier(10);
    case 2:
        return new BulletWallModifier();
    case 3:
        return new BulletBehaviourModifier(game::BulletCollisionFlag::PASS_THROUGH);

    default:
        break;
    }

    return nullptr;
}
