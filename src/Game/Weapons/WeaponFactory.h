
#pragma once

#include "IWeaponFactory.h"
#include "MonoFwd.h"
#include "WeaponConfiguration.h"

#include <unordered_map>

namespace game
{
    class WeaponFactory : public IWeaponFactory
    {
    public:
        WeaponFactory(mono::IEntityManager* entity_manager, mono::SystemContext* system_context);
        ~WeaponFactory();

        IWeaponPtr CreateWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner) override;

    private:

        IWeaponPtr CreateBulletWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id);
        IWeaponPtr CreateThrowableWeapon(WeaponSetup setup, WeaponFaction faction, uint32_t owner_id);

        mono::IEntityManager* m_entity_manager;
        mono::SystemContext* m_system_context;

        std::unordered_map<uint32_t, BulletImpactCallback> m_bullet_callbacks;

        std::unordered_map<uint32_t, struct BulletConfiguration> m_bullet_configs;
        std::unordered_map<uint32_t, struct WeaponConfiguration> m_weapon_configs;
    };
}
