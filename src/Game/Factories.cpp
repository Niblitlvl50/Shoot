
#include "Factories.h"

game::IWeaponFactory* game::g_weapon_factory = nullptr;
game::EntityLogicFactory* game::g_logic_factory = nullptr;
mono::IEntityManager* game::g_entity_manager = nullptr;
game::IDebugDrawer* game::g_debug_drawer = nullptr;
