
#pragma once

#include "Weapons/IWeaponSystem.h"
#include "Enemy.h"

namespace game
{
    class CacoDemonController : public IEnemyController
    {
    public:

        CacoDemonController(mono::EventHandler& event_handler);

        void Initialize(Enemy* enemy) override;
        void doUpdate(unsigned int delta) override;

    private:

        std::unique_ptr<IWeaponSystem> m_weapon;
        Enemy* m_enemy;
    };
}
