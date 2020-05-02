
#pragma once

#include "IWeaponFactory.h"
#include "MonoFwd.h"

class IEntityManager;

namespace game
{
    class WeaponFactory : public IWeaponFactory
    {
    public:
        WeaponFactory(IEntityManager* entity_manager, mono::SystemContext* system_context);
        std::unique_ptr<IWeaponSystem> CreateWeapon(WeaponType weapon, WeaponFaction faction, uint32_t owner) override;

    private:
        IEntityManager* m_entity_manager;
        mono::SystemContext* m_system_context;
    };
}
