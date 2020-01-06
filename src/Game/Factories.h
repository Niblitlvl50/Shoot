
#pragma once

class IEntityManager;

namespace game
{
    extern class IWeaponFactory* g_weapon_factory;
    extern class EntityLogicFactory* g_logic_factory;
    extern IEntityManager* g_entity_manager;
}
