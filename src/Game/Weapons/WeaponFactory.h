
#pragma once

#include "IWeaponFactory.h"
#include "MonoFwd.h"

namespace game
{
    class WeaponFactory : public IWeaponFactory
    {
    public:
        WeaponFactory(mono::IEntityManager* entity_manager, mono::SystemContext* system_context);
        ~WeaponFactory();

        IWeaponPtr CreateWeapon(WeaponType weapon, WeaponFaction faction, uint32_t owner) override;

    private:

        IWeaponPtr CreateBulletWeapon(WeaponType weapon_type, WeaponFaction faction, uint32_t owner_id);
        IWeaponPtr CreateThrowableWeapon(WeaponType weapon_type, WeaponFaction faction, uint32_t owner_id);

        mono::IEntityManager* m_entity_manager;
        mono::SystemContext* m_system_context;
    };
}
