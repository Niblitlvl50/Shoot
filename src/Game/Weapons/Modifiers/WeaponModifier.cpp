
#include "Weapons/IWeaponModifier.h"
#include <typeinfo>

uint32_t game::IWeaponModifier::Id() const
{
    return typeid(*this).hash_code();
}
